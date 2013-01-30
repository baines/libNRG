#include "nrg_player_impl.h"

using namespace nrg;

PlayerImpl::PlayerImpl(uint16_t id, const Server& s, const NetAddress& addr) 
: addr(addr), sock(s.getSocket()), in(addr), out(addr, sock), buffer(NRG_MAX_PACKET_SIZE), 
states(), handshake(), game_state(s.getSnapshot(), s.getDeltaSnapshots()) {
	states.push_back(&game_state);
	states.push_back(&handshake);
}

bool PlayerImpl::addPacket(Packet& p){
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

bool PlayerImpl::update(){
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

void PlayerImpl::kick(const char* reason){
	//TODO
}

int PlayerImpl::getPing() const {
	//TODO
	return 0;
}
