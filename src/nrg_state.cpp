#include "nrg_state.h"

nrg::ClientHandshakeState::ClientHandshakeState() : phase(NOT_STARTED){};

bool nrg::ClientHandshakeState::addIncomingPacket(Packet& p){
	uint8_t v = 0;
	if(phase != WAITING_ON_RESPONSE || p.size() < 1) return false;
	p.read8(v);
	if(v != 0) phase = static_cast<HandShakePhase>(v);
	return true;
}
	
bool nrg::ClientHandshakeState::needsUpdate() const {
	return (phase != WAITING_ON_RESPONSE);
}
	
nrg::StateUpdateResult nrg::ClientHandshakeState::update(ConnectionOutgoing& out){
	StateUpdateResult res;
	if(phase == NOT_STARTED){
		Packet p(1);
		p.write8(1);
		out.sendPacket(p);
		phase = WAITING_ON_RESPONSE;
		res = STATE_CONTINUE;
	} else if(phase == ACCEPTED){
		res = STATE_EXIT_SUCCESS;
	} else {
		res = STATE_EXIT_FAILURE;
	}
	return res;
}
