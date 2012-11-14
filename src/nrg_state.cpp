#include "nrg_state.h"
#include "nrg_config.h"
#include <climits>

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

nrg::ClientGameState::ClientGameState() : entities(), entity_types(){

}

bool nrg::ClientGameState::addIncomingPacket(Packet& p){
	if(p.size() < 2) return false;

	bool valid = false;
	uint16_t new_state_id = 0;
	p.read16(new_state_id);

	for(int i = 1; i < NRG_NUM_PAST_STATES; ++i){
		uint16_t id = (state_id + i) & USHRT_MAX;
		if(id == new_state_id){
			valid = true;
			break;
		}
	}
	if(!valid) return false;

	// get last acknowledged input.

	// get all entity changes and apply them
	return true;
}

void nrg::ClientGameState::registerEntity(Entity* e){
	entity_types.insert(std::pair<uint16_t, Entity*>(e->getID(), e));
}