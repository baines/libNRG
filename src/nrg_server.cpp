#include "nrg_server.h"
#include "nrg_config.h"
#include "nrg_player_impl.h"
#include "nrg_os.h"

using namespace std;
using namespace nrg;

namespace {
	typedef map<NetAddress, Player*> ClientMap;
	
	static void clearEntityUpdated(Entity* e){
		e->markUpdated(false);
	
		for(FieldBase* f = e->getFirstField(); f; f = f->getNextField()){
			f->setUpdated(false);
		}
	}
	
}

Server::Server(const char* game_name, uint32_t game_version, InputBase& input) 
: sock()
, buffer()
, input(&input)
, eventq()
, clients()
, timer(nrg::os::microseconds())
, interval(NRG_DEFAULT_SERVER_INTERVAL_US){

}

Server::Server(const char* game_name, uint32_t game_version) 
: sock()
, buffer()
, input(nullptr)
, eventq()
, clients()
, timer(nrg::os::microseconds())
, interval(NRG_DEFAULT_SERVER_INTERVAL_US){

}

bool Server::bind(const NetAddress& addr){
	sock.setFamilyFromAddress(addr);
	sock.setNonBlocking(true);
	
	return sock.bind(addr);
}

bool Server::isBound() {
	return sock.getBoundAddress() != NULL;
}

size_t Server::playerCount() const {
	return clients.size();
}

static inline PlayerImpl* IMPL(nrg::Player* p){
	return static_cast<nrg::PlayerImpl*>(p);
}

bool Server::update(){
	eventq.clear();

	uint64_t blocktime;

	while(blocktime = max<int>(0, interval - (os::microseconds() - timer)),
	sock.dataPending(blocktime)){
		
		NetAddress addr;
		sock.recvPacket(buffer.reset(), addr);
		
		ClientMap::iterator it = clients.find(addr);
		if(it == clients.end()){
			printf("new client: %s:%d\n", addr.name(), addr.port());
			
			uint16_t pid = player_ids.acquire();
			auto res = clients.insert(make_pair(addr, nullptr));
			res.first->second = new PlayerImpl(pid, *this, res.first->first);
			
			PlayerEvent e = { PLAYER_JOIN, pid, res.first->second };
			eventq.pushEvent(e);
			
			it = res.first;
		}
		
		if(!IMPL(it->second)->addPacket(buffer)){
			it->second->kick("Recieved invalid packet from client.");
		}
	}

	timer = os::microseconds();

	// generate snapshot
	DeltaSnapshot& delta_ss = snaps.next();
	delta_ss.reset();
	delta_ss.setID(timer / 1000);
	master_snapshot.setID(timer / 1000);
	
	sort(updated_entities.begin(), updated_entities.end());
	
	for(uint16_t i : updated_entities){
		if(entities.size() > i){
			Entity* e = entities[i];
			
			if(e != NULL){
				master_snapshot.addEntity(e);
				delta_ss.addEntity(e);
				clearEntityUpdated(e);
			} else {
				master_snapshot.removeEntityById(i);
				delta_ss.removeEntityById(i);
			}
		}
	}
	updated_entities.clear();

	for(ClientMap::iterator i = clients.begin(), j = clients.end(); i != j; /**/){
		if(!i->second->isConnected()){
			printf("client quit: %s:%d\n", i->first.name(), i->first.port());
			
			player_ids.release(i->second->getID());
			PlayerEvent e = { PLAYER_LEAVE, i->second->getID(), i->second };
			eventq.pushEvent(e);

			delete i->second;
			clients.erase(i++);
		} else {
			++i;
		}
	}

	for(auto& c : clients){
		if(!IMPL(c.second)->update()){
			c.second->kick("Client update failed.");
		}
	}
	return true;
}

bool Server::pollEvent(Event& e){
	return eventq.pollEvent(e);
}

void Server::registerEntity(Entity& e){
	e.setManager(this);
	uint16_t id = entity_ids.acquire();
	e.setID(id);
	if(entities.size() <= id){
		entities.resize(id+1);
	}
	entities[id] = &e;
	updated_entities.push_back(e.getID());
}

void Server::unregisterEntity(Entity& e){
	if(entities[e.getID()]){
		entity_ids.release(e.getID());
		updated_entities.push_back(e.getID());
		entities[e.getID()] = nullptr;
	}
}

void Server::markEntityUpdated(Entity& e){
	if(find(updated_entities.begin(), updated_entities.end(), e.getID()) 
	== updated_entities.end()){
		updated_entities.push_back(e.getID());
	}
}

Player* Server::getPlayerByID(uint16_t id) const {
	auto i = find_if(clients.begin(), clients.end(), 
	[&](const pair<NetAddress, Player*>& p){
		return p.second->getID() == id;
	});
	return i == clients.end() ? nullptr : i->second;
}

Server::~Server(){
	for(auto& c : clients){
		c.second->kick("Server closing.");
		delete c.second;
	}

	for(Entity* e : entities){
		if(e){
			e->setManager(nullptr);
			e->setID(0);
		}
	}
}
