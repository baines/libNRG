#include "nrg_server.h"
#include "nrg_config.h"
#include "nrg_field_impl.h"
#include "nrg_player_impl.h"
#include "nrg_os.h"

nrg::Server::Server(const NetAddress& bind_addr) 
: sock(), buffer(NRG_MAX_PACKET_SIZE), eventq(), clients(), 
timer(nrg::os::microseconds()), interval(50000) {
	sock.setNonBlocking(true);
	sock.bind(bind_addr);
}

void nrg::Server::bind(const NetAddress& addr){
	sock.bind(addr);
}

bool nrg::Server::isBound() {
	return sock.getBoundAddress() != NULL;
}

size_t nrg::Server::playerCount() const {
	return clients.size();
}

void nrg::Server::clearEntityUpdated(Entity* e, FieldList& fl){
	e->nrg_updated = false;
	
	for(size_t i = 0; i < fl.size(); ++i){
		fl.get(i)->setUpdated(false);
	}
}

static inline nrg::PlayerImpl* IMPL(nrg::Player* p){
	return static_cast<nrg::PlayerImpl*>(p);
}

nrg::status_t nrg::Server::update(){
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
			std::pair<ClientMap::iterator, bool> res = clients.insert(
				std::pair<NetAddress, PlayerImpl*>(addr, NULL)
			);
			res.first->second = new PlayerImpl(pid, *this, res.first->first);
			
			PlayerEvent e = { PLAYER_JOIN, pid, res.first->second };
			eventq.pushEvent(e);
			
			it = res.first;
		}
		
		if(!IMPL(it->second)->addPacket(buffer)){
			// kick client
		}
		blocktime = std::max<int>(0, interval - (os::microseconds() - timer));
	}

	timer = os::microseconds();

	// generate snapshot
	DeltaSnapshot& delta_ss = snaps.next();
	master_snapshot.setID(delta_ss.getID());
	
	FieldListImpl fl;

	for(std::set<uint16_t>::iterator i = updated_entities.begin(),
	j = updated_entities.end(); i != j; ++i){
		if(entities.size() > *i){
			if(entities[*i] != NULL){
				fl.vec.clear();
				entities[*i]->getFields(fl);
				master_snapshot.addEntity(entities[*i], fl);
				delta_ss.addEntity(entities[*i], fl);
				clearEntityUpdated(entities[*i], fl);
			} else {
				master_snapshot.removeEntityById(*i);
				delta_ss.removeEntityById(*i);
			}
		}
	}
	updated_entities.clear();

	for(ClientMap::iterator i = clients.begin(), j = clients.end(); i != j; ++i){
		if(!IMPL(i->second)->update()){
			// kick client
		}
	}
	return status::OK;
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
	updated_entities.insert(e->getID());
}

void nrg::Server::unregisterEntity(Entity* e){
	if(e && entities[e->nrg_id]){
		entity_ids.release(e->nrg_id);
		updated_entities.insert(e->nrg_id);
		entities[e->nrg_id] = NULL;
	}
}

void nrg::Server::markEntityUpdated(Entity* e){
	if(e) updated_entities.insert(e->getID());
}

nrg::Server::~Server(){
	for(std::vector<Entity*>::iterator i = entities.begin(), j = entities.end()
	; i != j; ++i){
		if((*i)){
			(*i)->nrg_serv_ptr = NULL;
			(*i)->nrg_id = 0;
		}
	}
}
