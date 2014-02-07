#include "nrg_client_state.h"
#include "nrg_input.h"
#include "nrg_config.h"
#include "nrg_os.h"
#include "nrg_clientstats_impl.h"
#include <climits>

using namespace nrg;

namespace {
	enum {
		HS_NOT_STARTED = -1,
		HS_WAITING     = 0,
		HS_CLIENT_SYN  = 1,
		HS_ACCEPTED    = 1	
	};
	
	static const size_t NRG_CGS_HEADER_SIZE = 8;
	typedef std::vector<Entity*>::iterator e_it;
	typedef std::vector<FieldBase*>::iterator f_it;
	typedef std::map<uint16_t, Entity*>::iterator et_it;
}

ClientHandshakeState::ClientHandshakeState() 
: phase(HS_NOT_STARTED)
, timeouts(0){

}

bool ClientHandshakeState::onRecvPacket(Packet& p, PacketFlags f){
	if(phase == HS_WAITING && p.remaining()){
		uint8_t i = 0;
		p.read8(i);
		phase = i;
		return true;
	} else {
		return false;
	}
}
	
bool ClientHandshakeState::needsUpdate() const {
	return (phase != HS_WAITING);
}
	
StateResult ClientHandshakeState::update(ConnectionOut& out, StateFlags f){
	StateResult res;
	
	if(f & SFLAG_TIMED_OUT){
		if(++timeouts > 5){
			res = STATE_FAILURE;
		} else {
			out.resendLastPacket();
			res = STATE_CONTINUE;
		}
	} else {
		timeouts = 0;
		if(phase == HS_NOT_STARTED){
			buffer.reset().write8(HS_CLIENT_SYN);
			out.sendPacket(buffer);
			phase = HS_WAITING;
			res = STATE_CONTINUE;
		} else if(phase == HS_ACCEPTED){
			res = STATE_CHANGE;
		} else {
			res = STATE_FAILURE;
		}
	}
	
	return res;
}

ClientHandshakeState::~ClientHandshakeState(){

}

ClientGameState::ClientGameState(EventQueue& eq, const Socket& s, Input& i) 
: entities()
, entity_types()
, client_eventq(eq)
, stats(new ClientStatsImpl())
, state_id(-1)
, timeouts(0)
, ss_timer(0.0)
, s_time_ms(0)
, c_time0_ms(0)
, c_time_ms(0)
, snapshot()
, buffer()
, sock(s)
, input(i)
, replay() {

}

bool ClientGameState::onRecvPacket(Packet& p, PacketFlags f){
#ifdef NRG_TEST_PACKET_DROP
	static bool drop = true;
	drop = rand()%4 == 0;
	if(drop){
		puts("dropping this packet.");		
		return true;
	}
#endif

	ClientStatsImpl& stats_impl = *static_cast<ClientStatsImpl*>(stats);

	if(replay.isRecording()) replay.addPacket(p);

	if(p.size() < NRG_CGS_HEADER_SIZE) return false;

	uint16_t new_state_id = 0;
	p.read16(new_state_id);

	uint16_t dropped = new_state_id - state_id;
	if(state_id != -1){
		if (dropped >= NRG_NUM_PAST_SNAPSHOTS)	return false;
		while(--dropped) stats_impl.addSnapshotStat(-1);
	}

	uint32_t s_newtime_ms = 0;
	p.read32(s_newtime_ms);
	c_time0_ms = c_time_ms;
	c_time_ms = sock.getLastTimestamp() / 1000;

	if(state_id != -1){
		c_time_ms = std::min(c_time_ms, c_time0_ms + (s_newtime_ms - s_time_ms));
	}
	s_time_ms = s_newtime_ms;

	uint16_t ping = 0;
	p.read16(ping);
	stats_impl.addSnapshotStat(ping);

	uint16_t ackd_input_id = 0;
	p.read16(ackd_input_id);
	
	snapshot.reset();
	if(!snapshot.readFromPacket(p)) return false;
	state_id = new_state_id;

	// Mark all previously updated entities as no longer updated
	// Could maybe store fields instead of entities for better efficiency
	for(e_it i = entities.begin(), j = entities.end(); i != j; ++i){
		(*i)->nrg_updated = false;
		for(FieldBase* f = (*i)->getFirstField(); f; f = f->getNextField()){
			f->shiftData();			
			f->setUpdated(false);
		}
	}

	snapshot.applyUpdate(entities, entity_types, client_eventq);

	return true;
}

bool ClientGameState::needsUpdate() const {
	return state_id != -1;
}
	
StateResult ClientGameState::update(ConnectionOut& out, StateFlags f){
	if(f & SFLAG_TIMED_OUT){
		if(++timeouts > 5){
			return STATE_FAILURE;
		} else {
			out.resendLastPacket();
			return STATE_CONTINUE;
		}
	}
	
	timeouts = 0;
	uint32_t now_ms = os::microseconds() / 1000;
	ss_timer = ((now_ms-50)-c_time0_ms) / double(c_time_ms - c_time0_ms);
	uint32_t s_time_est = s_time_ms + (now_ms - c_time_ms);

	static_cast<ClientStatsImpl*>(stats)->addInterpStat(ss_timer <= 1.0 ? 1 : ss_timer);

	buffer.reset().write16(state_id).write32(s_time_est);
	input.writeToPacket(buffer);
	out.sendPacket(buffer);

	return STATE_CONTINUE;
}

void ClientGameState::registerEntity(Entity* e){
	e->nrg_cgs_ptr = this;
	entity_types.insert(std::make_pair(e->getType(), e));
}

void ClientGameState::registerMessage(const MessageBase& m){
	messages.insert(std::make_pair(m.getID(), m.clone()));
}

double ClientGameState::getSnapshotTiming() const {
	return ss_timer;
}

const ClientStats& ClientGameState::getClientStats() const {
	return *stats;
}

void ClientGameState::startRecordingReplay(const char* filename) {
	replay.startRecording(filename, state_id, entities);
}

void ClientGameState::stopRecordingReplay() {
	replay.stopRecording();
}

ClientGameState::~ClientGameState(){
	if(replay.isRecording()) replay.stopRecording();
	delete stats;
	for(e_it i = entities.begin(), j = entities.end(); i != j; ++i){
		if(*i){
			delete *i;
		}
	}
	for(et_it i = entity_types.begin(), j = entity_types.end(); i != j; ++i){
		delete i->second;
	}
}
