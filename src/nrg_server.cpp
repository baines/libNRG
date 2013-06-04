#include "nrg_server.h"
#include "nrg_config.h"
#include "nrg_player_impl.h"
#include "nrg_os.h"

using namespace std;

nrg::Server::Server(const NetAddress& bind_addr, Input& input) 
: sock(), buffer(), input(input), eventq(), clients(), 
timer(nrg::os::microseconds()), interval(50000) {
	sock.setNonBlocking(true);
	bind(bind_addr);
}

bool nrg::Server::bind(const NetAddress& addr){
	return sock.bind(addr) == status::OK;
}

bool nrg::Server::isBound() {
	return sock.getBoundAddress() != NULL;
}

size_t nrg::Server::playerCount() const {
	return clients.size();
}

void nrg::Server::clearEntityUpdated(Entity* e){
	e->nrg_updated = false;
	
	for(FieldBase* f = e->getFirstField(); f; f = f->getNextField()){
		f->setUpdated(false);
	}
}

static inline nrg::PlayerImpl* IMPL(nrg::Player* p){
	return static_cast<nrg::PlayerImpl*>(p);
}

bool nrg::Server::update(){
	eventq.clear();

	uint64_t blocktime = std::max<int>(0, interval - (os::microseconds() - timer));

	while(sock.dataPending(blocktime)){
		NetAddress addr;
		buffer.reset();
		sock.recvPacket(buffer, addr);
		
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
		blocktime = std::max<int>(0, interval - (os::microseconds() - timer));
	}

	timer = os::microseconds();

	// generate snapshot
	DeltaSnapshot& delta_ss = snaps.next();
	master_snapshot.setID(delta_ss.getID());
	
	sort(updated_entities.begin(), updated_entities.end());
	
	for(std::vector<uint16_t>::iterator i = updated_entities.begin(),
	j = updated_entities.end(); i != j; ++i){
		if(entities.size() > *i){
			if(entities[*i] != NULL){
				master_snapshot.addEntity(entities[*i]);
				delta_ss.addEntity(entities[*i]);
				clearEntityUpdated(entities[*i]);
			} else {
				master_snapshot.removeEntityById(*i);
				delta_ss.removeEntityById(*i);
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

	for(ClientMap::iterator i = clients.begin(), j = clients.end(); i != j; ++i){
		if(!IMPL(i->second)->update()){
			i->second->kick("Client update failed.");
		}
	}
	return true;
}

bool nrg::Server::pollEvent(Event& e){
	return eventq.pollEvent(e);
}

void nrg::Server::registerEntity(Entity* e){
	e->nrg_serv_ptr = this;
	uint16_t id = entity_ids.acquire();
	e->nrg_id = id;
	if(entities.size() <= id){
		entities.resize(id+1);
	}
	entities[id] = e;
	updated_entities.push_back(e->getID());
}

void nrg::Server::unregisterEntity(Entity* e){
	if(e && entities[e->nrg_id]){
		entity_ids.release(e->nrg_id);
		updated_entities.push_back(e->nrg_id);
		entities[e->nrg_id] = NULL;
	}
}

void nrg::Server::markEntityUpdated(Entity* e){
	if(e && std::find(updated_entities.begin(), updated_entities.end(), 
	e->getID()) == updated_entities.end()){
		updated_entities.push_back(e->getID());
	}
}

nrg::Player* nrg::Server::getPlayerByID(uint16_t id) const {
	for(ClientMap::const_iterator i = clients.begin(), j = clients.end(); i != j; ++i){
		if(i->second->getID() == id){
			return i->second;		
		}
	}
	return NULL;
}

nrg::Server::~Server(){
	for(ClientMap::iterator i = clients.begin(), j = clients.end(); i != j; ++i){
		i->second->kick("Server closing.");
		delete i->second;
	}

	for(std::vector<Entity*>::iterator i = entities.begin(), j = entities.end()
	; i != j; ++i){
		if((*i)){
			(*i)->nrg_serv_ptr = NULL;
			(*i)->nrg_id = 0;
		}
	}
}
