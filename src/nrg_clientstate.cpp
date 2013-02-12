#include "nrg_state.h"
#include "nrg_input.h"
#include "nrg_config.h"
#include "nrg_os.h"
#include "nrg_field_impl.h"
#include <climits>

#include <sys/ioctl.h>
#include <sys/time.h>

using namespace nrg;

ClientHandshakeState::ClientHandshakeState() : phase(NOT_STARTED){};

bool ClientHandshakeState::addIncomingPacket(Packet& p){
	uint8_t v = 0;
	if(phase != WAITING_ON_RESPONSE || p.size() < 1) return false;
	p.read8(v);
	if(v != 0) phase = static_cast<HandShakePhase>(v);
	return true;
}
	
bool ClientHandshakeState::needsUpdate() const {
	return (phase != WAITING_ON_RESPONSE);
}
	
StateUpdateResult ClientHandshakeState::update(ConnectionOutgoing& out){
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

ClientHandshakeState::~ClientHandshakeState(){

}

ClientGameState::ClientGameState(EventQueue& eq, const Socket& s, Input& i) 
: entities(), entity_types(), client_eventq(eq), state_id(-1), 
s_time_ms(0), c_time_ms(0), snapshot(), buffer(), sock(s), input(i) {

}

static const size_t NRG_CGS_HEADER_SIZE = 8;
typedef std::vector<Entity*>::iterator e_it;
typedef std::vector<FieldBase*>::iterator f_it;
typedef std::map<uint16_t, Entity*>::iterator et_it;

bool ClientGameState::addIncomingPacket(Packet& p){
	if(p.size() < NRG_CGS_HEADER_SIZE) return false;

	bool valid = false;

	uint16_t new_state_id = 0;
	p.read16(new_state_id);

	if(state_id == -1){
		valid = true;
	} else {
		for(int i = 1; i < NRG_NUM_PAST_SNAPSHOTS; ++i){
			uint16_t id = (state_id + i) & USHRT_MAX;
			if(id == new_state_id){
				valid = true;
				break;
			}
		}
	}
	if(!valid) return false;

	p.read32(s_time_ms);
	c_time_ms = sock.getLastTimestamp() / 1000;

	uint16_t ackd_input_id = 0;
	p.read16(ackd_input_id);
	// TODO: acknowledge input

	snapshot.reset();
	if(!snapshot.readFromPacket(p)) return false;
	state_id = new_state_id;

	// Mark all previously updated entities as no longer updated
	// Could maybe store fields instead of entities for better efficiency
	FieldListImpl fl;
	for(e_it i = entities.begin(), j = entities.end(); i != j; ++i){
		if(*i){
			(*i)->getFields(fl);
			(*i)->nrg_updated = false;
		}
	}

	for(size_t i = 0; i < fl.size(); ++i){
		fl.vec[i]->shiftData();
		fl.vec[i]->setUpdated(false);
	}

	// TODO timing of applying new snapshot
	snapshot.applyUpdate(entities, entity_types, client_eventq);

	return true;
}

bool ClientGameState::needsUpdate() const {
	//TODO, true if new input to send, or new snapshot to ack
	return state_id != -1;
}
	
StateUpdateResult ClientGameState::update(ConnectionOutgoing& out){
	uint32_t s_time_est = s_time_ms + ((os::microseconds() / 1000) - c_time_ms);
	buffer.reset().write16(state_id).write32(s_time_est);
	input.writeToPacket(buffer);
	out.sendPacket(buffer);

	return STATE_CONTINUE;
}

void ClientGameState::registerEntity(Entity* e){
	e->nrg_cgs_ptr = this;
	entity_types.insert(std::make_pair(e->getType(), e));
}

double ClientGameState::getSnapshotTiming() const {
	//FIXME: hardcoded 50ms interval assumption, get it during handshake instead?
	return ((os::microseconds() / 1000) - c_time_ms) / 50.0;
}

ClientGameState::~ClientGameState(){
	for(e_it i = entities.begin(), j = entities.end(); i != j; ++i){
		if(*i){
			delete *i;
		}
	}
	for(et_it i = entity_types.begin(), j = entity_types.end(); i != j; ++i){
		delete i->second;
	}
}
