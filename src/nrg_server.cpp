#include "nrg_server.h"
#include "nrg_config.h"
#include "nrg_os.h"

nrg::Server::Server(const NetAddress& bind_addr) 
: sock(), buffer(NRG_MAX_PACKET_SIZE), clients(), timer(nrg::os::microseconds()),
interval(50000) {
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

nrg::status_t nrg::Server::update(){
	uint64_t delta = std::max<int>(0, interval - (os::microseconds() - timer));

	while(sock.dataPending(delta / 1000)){
		NetAddress addr;
		buffer.reset();
		sock.recvPacket(buffer, addr);
		
		ClientMap::iterator it = clients.find(addr);
		if(it == clients.end()){
			std::pair<ClientMap::iterator, bool> res = clients.insert(
				std::pair<NetAddress, PlayerConnection*>(addr, NULL)
			);
			res.first->second = new PlayerConnection(master_snapshot, sock, res.first->first);
			it = res.first;
		}
		
		if(!it->second->addPacket(buffer)){
			// kick client
		}
		delta = std::max<int>(0, interval - (os::microseconds() - timer));
	}

	timer = os::microseconds();

	// generate snapshot
	master_snapshot.resetAndIncrement();

	for(std::vector<Entity*>::iterator i = updated_entities.begin(),
	j = updated_entities.end(); i != j; ++i){
		master_snapshot.addEntity((*i));
		(*i)->nrg_updated = false;
	}
	updated_entities.clear();

	for(ClientMap::iterator i = clients.begin(), j = clients.end(); i != j; ++i){
		if(!i->second->update()){
			// kick client
		}
	}
	return status::OK;
}

// TODO reuse removed entity IDs
static uint16_t getNextEntityId(void){
	static uint16_t id = 0;
	return id++;
}

void nrg::Server::registerEntity(Entity* e){
	e->nrg_serv_ptr = this;
	e->nrg_id = getNextEntityId();
}

void nrg::Server::markEntityUpdated(Entity* e){
	updated_entities.push_back(e);
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

nrg::PlayerConnection::PlayerConnection(const Snapshot& ss, 
const UDPSocket& sock, const NetAddress& addr) : addr(addr), sock(sock), 
in(addr), out(addr, sock), buffer(NRG_MAX_PACKET_SIZE), states(), 
handshake(), game_state(ss) {
	states.push_back(&game_state);
	states.push_back(&handshake);
}

bool nrg::PlayerConnection::addPacket(Packet& p){
	bool valid = false;
	if(in.addPacket(p)){
		if(in.hasNewPacket()){
			buffer.reset();
			in.getLatestPacket(buffer);
			valid = states.back()->addIncomingPacket(buffer);
		} else {
			valid = true;
		}
	}
	return valid;
}

bool nrg::PlayerConnection::update(){
	if(states.empty()) return false;
	
	if(states.back()->needsUpdate()){
		StateUpdateResult ur = states.back()->update(out);
		if(ur != STATE_CONTINUE){
			states.pop_back();
		}
		return ur == STATE_EXIT_SUCCESS;
	} else {
		return true;
	}
}
