#include "nrg_server.h"
#include "nrg_config.h"

nrg::Server::Server(const NetAddress& bind_addr) 
: sock(), buffer(NRG_MAX_PACKET_SIZE), clients() {
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
	while(sock.dataPending()){
		NetAddress addr;
		buffer.reset();
		sock.recvPacket(buffer, addr);
		
		ClientMap::iterator it = clients.find(addr);
		if(it == clients.end()){
			std::pair<ClientMap::iterator, bool> res = clients.insert(
				std::pair<NetAddress, PlayerConnection*>(addr, NULL)
			);
			res.first->second = new PlayerConnection(sock, res.first->first);
		} else if(!it->second->addPacket(buffer)){
			// kick client
		}
	}

	for(ClientMap::iterator i = clients.begin(), j = clients.end(); i != j; ++i){
		if(!i->second->update()){
			// kick client
		}
	}
	return status::OK;
}

nrg::PlayerConnection::PlayerConnection(const UDPSocket& sock, const NetAddress& addr)
: addr(addr), sock(sock), in(addr), out(addr, sock), buffer(NRG_MAX_PACKET_SIZE), states(){

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
