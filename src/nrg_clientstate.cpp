#include "nrg_state.h"
#include "nrg_config.h"
#include "nrg_field_impl.h"
#include <climits>

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

ClientGameState::ClientGameState(EventQueue& eq) : entities(), updated_entities(),
entity_types(), client_eventq(eq), state_id(-1), snapshot(){

}

static const size_t NRG_CGS_HEADER_SIZE = 4;
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

	uint16_t ackd_input_id = 0;
	p.read16(ackd_input_id);
	// TODO: acknowledge input

	next_snapshot.reset();
	if(!next_snapshot.readFromPacket(p)) return false;
	state_id = new_state_id;

	// Mark all previously updated entities as no longer updated
	// Could maybe store fields instead of entities for better efficiency
	FieldListImpl fl;
	for(e_it i = updated_entities.begin(), j = updated_entities.end(); i != j; ++i){
		fl.vec.clear();
		(*i)->nrg_updated = false;
		(*i)->getFields(fl);
		for(f_it k = fl.vec.begin(), l = fl.vec.end(); k != l; ++k){
			(*k)->setUpdated(false);
		}
	}
	updated_entities.clear();

	// TODO timing of applying new snapshot
	EventQueue eq;
	snapshot.applyUpdate(entities, entity_types, eq);

	Event e;
	while(eq.pollEvent(e)){
		if(e.type == ENTITY_UPDATED){
			updated_entities.push_back(e.entity.pointer);
		}
		client_eventq.pushEvent(e);
	}

	snapshot = next_snapshot;

	return true;
}

bool ClientGameState::needsUpdate() const {
	//TODO, true if new input to send, or new snapshot to ack
	return state_id != -1;
}
	
StateUpdateResult ClientGameState::update(ConnectionOutgoing& out){
	//TODO collect and send input to server w/ last recieved snapshot id
	buffer.reset().write16(state_id);
	out.sendPacket(buffer);
	return STATE_CONTINUE;
}

void ClientGameState::registerEntity(Entity* e){
	entity_types.insert(std::make_pair(e->getType(), e));
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
