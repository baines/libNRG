#include "nrg_player_impl.h"

using namespace nrg;

PlayerImpl::PlayerImpl(uint16_t id, const Server& s, const NetAddress& addr) 
: server(s), addr(addr), sock(s.getSocket()), in(addr), out(addr, sock), 
buffer(NRG_MAX_PACKET_SIZE), states(), handshake(), game_state(s.getSnapshot(), 
s.getDeltaSnapshots(), s.getInput(), *this, ping), id(id), connected(true), ping(0) {
	states.push_back(&game_state);
	states.push_back(&handshake);
}

bool PlayerImpl::addPacket(Packet& p){
	bool valid = false;
	if(!states.empty() && in.addPacket(p)){
		if(in.hasNewPacket()){
			in.getLatestPacket(buffer.reset());
			if(in.isLatestPacketFinal()){
				valid = true;				
				connected = false;
				states.clear();
			} else {
				valid = states.back()->addIncomingPacket(buffer);
			}
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
		return ur != STATE_EXIT_FAILURE;
	} else {
		return true;
	}
}

void PlayerImpl::kick(const char* reason){
	buffer.reset().writeArray((uint8_t*)reason, strlen(reason));
	out.sendDisconnect(buffer);
	connected = false;
}

bool PlayerImpl::isConnected() const {
	return connected;
}

int PlayerImpl::getPing() const {
	return ping;
}
