#include "nrg_client.h"
#include "nrg_config.h"

struct TestState : public nrg::State {
	TestState(nrg::ConnectionOutgoing& out) : State(out){};
	bool addIncomingPacket(nrg::Packet& p){ return true; }
	bool needsUpdate() const { return true; }	
	int update(){
		nrg::Packet p(1500);
		for(int i = 0; i < 1400; ++i)
			p.write8(0x42);
		out.sendPacket(p);
	}
};

nrg::Client::Client(const NetAddress& addr) 
: sock(), serv_addr(addr), in(serv_addr), out(serv_addr, sock), states() {
	states.push_back(new TestState(out));
	sock.setNonBlocking(true);
}

bool nrg::Client::sendUpdate(){
	if(states.back()->needsUpdate()){
		states.back()->update();
		return true;
	}
	return false;
}

bool nrg::Client::recvUpdate(){
	if(sock.dataPending()){
		Packet p(NRG_MAX_PACKET_SIZE);
		NetAddress addr;
		sock.recvPacket(p, addr); 
	}
}
