#include "nrg_client.h"
#include "nrg_config.h"

nrg::Client::Client(const NetAddress& addr) : sock(), buffer(NRG_MAX_PACKET_SIZE), 
serv_addr(addr), in(serv_addr), out(serv_addr, sock), states() {
	sock.setNonBlocking(true);
	states.push_back(new ClientHandshakeState(out));
}

nrg::status_t nrg::Client::update(){
	if(states.empty()){
		return status::ERROR;
	}
	
	while(sock.dataPending()){
		NetAddress addr, *bound_addr = sock.getBoundAddress();
		buffer.reset();
		sock.recvPacket(buffer, addr);
		if(bound_addr != NULL && addr != *bound_addr) continue;
		states.back()->addIncomingPacket(buffer);
	}
	
	if(states.back()->needsUpdate()){
		State::UpdateResult ur = states.back()->update();
		
		if(ur != State::STATE_CONTINUE){
			states.pop_back();
		}
		
		if(ur == State::STATE_EXIT_FAILURE){
			return status::ERROR;	
		}
	}
	return status::OK;
}
