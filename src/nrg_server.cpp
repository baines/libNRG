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
			clients[addr] = new PlayerConnection(sock, addr);
		} else if(!it->second->addPacket(buffer)){
			// kick client
		}
	}

	for(ClientMap::iterator i = clients.begin(), j = clients.end(); i != j; ++i){
		i->second->update();
	}
	return status::OK;
}


