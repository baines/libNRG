#include "nrg_client.h"
#include "nrg_config.h"

nrg::Client::Client(const NetAddress& addr) : sock(), buffer(NRG_MAX_PACKET_SIZE), 
serv_addr(addr), in(serv_addr), out(serv_addr, sock), states(), handshake() {
	sock.setNonBlocking(true);
	states.push_back(&handshake);
}

nrg::status_t nrg::Client::update(){
	if(states.empty()){
		return status::ERROR;
	}
	
	while(sock.dataPending()){
		NetAddress addr;
		buffer.reset();
		sock.recvPacket(buffer, addr);
		//if(addr != serv_addr) continue;
		if(in.addPacket(buffer) && in.hasNewPacket()){
			buffer.reset();
			in.getLatestPacket(buffer);
			states.back()->addIncomingPacket(buffer);
		}
	}
	
	if(states.back()->needsUpdate()){
		StateUpdateResult ur = states.back()->update(out);
		
		if(ur != STATE_CONTINUE){
			states.pop_back();
		}
		
		if(ur == STATE_EXIT_FAILURE){
			return status::ERROR;	
		}
	}
	return status::OK;
}
