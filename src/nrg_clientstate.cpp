#include "nrg_client_state.h"
#include "nrg_client.h"
#include "nrg_input.h"
#include "nrg_config.h"
#include "nrg_os.h"
#include "nrg_varint.h"
#include "nrg_clientstats_impl.h"
#include <climits>
#include <cassert>
#include <cmath>

using namespace nrg;
using namespace std;

namespace {
	enum {
		HS_NOT_STARTED = -1,
		HS_WAITING     = 0,
		HS_CLIENT_SYN  = 1,
		HS_ACCEPTED    = 1	
	};
	
	static const size_t NRG_CGS_HEADER_SIZE = 4;
	typedef vector<Entity*>::iterator e_it;
	typedef vector<FieldBase*>::iterator f_it;
	typedef map<uint16_t, Entity*>::iterator et_it;
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

ClientGameState::ClientGameState() 
: entities()
, entity_types()
, client_eventq(nullptr)
, stats(new ClientStatsImpl())
, timeouts(0)
, ss_timer(0.0)
, server_seq_prev(0)
, server_ms_prev(0)
, client_ms(0)
, client_ms_prev(0)
, interval(0)
, snapshot()
, buffer()
, input(nullptr)
, replay()
, got_packet(false)
, client(nullptr) {

}

bool ClientGameState::init(Client* c, Server* s, Player* p){
	assert(c != nullptr);

	client = c;
	input = c->getInput();
	client_eventq = &c->getEventQueue();
	
	return true;
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
	ClientStatsImpl& stats_impl = *static_cast<ClientStatsImpl*>(stats.get());
	
	if(replay.isRecording()) replay.addPacket(p);
	if(f & PKTFLAG_OUT_OF_ORDER) return true;
	if(p.size() < NRG_CGS_HEADER_SIZE) return false;

	uint16_t server_ms = 0;
	p.read16(server_ms);
	
	client_ms_prev = client_ms;
	client_ms = client->getSock().getLastTimestamp() / 1000;
	
	uint8_t server_seq = 0;
	p.read8(server_seq);

	if(got_packet){
		int dropped = server_seq - server_seq_prev;
		interval = uint16_t(server_ms - server_ms_prev) / uint8_t(server_seq - server_seq_prev);
		while(--dropped > 0) stats_impl.addSnapshotStat(-1);
	}
	server_ms_prev = server_ms;
	server_seq_prev = server_seq;

	UVarint ping_vi;
	ping_vi.decode(p);
	
	stats_impl.addSnapshotStat(ping_vi.get());
	
	// Mark all previously updated entities as no longer updated
	// Could maybe store fields instead of entities for better efficiency
	
	//TODO: update this when fields.get() is called rather than here.
	for(e_it i = entities.begin(), j = entities.end(); i != j; ++i){
		if(*i == nullptr) continue;
		
		(*i)->markUpdated(false);
		for(FieldBase* f = (*i)->getFirstField(); f; f = f->getNextField()){
			f->shiftData();			
			f->setUpdated(false);
		}
	}
	
	snapshot.reset();
	if(!snapshot.readFromPacket(p)) return false;
	snapshot.applyUpdate(entities, entity_types, *client_eventq);

	got_packet = true;
	return true;
}

bool ClientGameState::needsUpdate() const {
	return got_packet;
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
	
	uint32_t now_ms = os::milliseconds();
	int32_t n = max<int32_t>((now_ms - interval) - client_ms_prev, 0);
	ss_timer = n / double(client_ms - client_ms_prev);
	
	static_cast<ClientStatsImpl*>(stats.get())->addInterpStat(ss_timer <= 1.0 ? 1 : ss_timer);

	buffer.reset().write16(server_ms_prev);
	TVarint<uint16_t>(now_ms - client_ms).encode(buffer);
	if(input) input->writeToPacket(buffer);
	out.sendPacket(buffer);

	return STATE_CONTINUE;
}

void ClientGameState::registerEntity(Entity* e){
	e->setManager(this);
	entity_types.insert(make_pair(e->getType(), e));
}

void ClientGameState::registerMessage(const MessageBase& m){
	messages.insert(make_pair(m.getID(), m.clone()));
}

void ClientGameState::registerMessage(MessageBase&& m){
	messages.insert(make_pair(m.getID(), m.move_clone()));
}

float ClientGameState::getInterpTimer() const {
	return ss_timer;
}

const ClientStats& ClientGameState::getClientStats() const {
	return *stats;
}

void ClientGameState::startRecordingReplay(const char* filename) {
	//XXX: fix replays! replay.startRecording(filename, state_id, entities);
}

void ClientGameState::stopRecordingReplay() {
	replay.stopRecording();
}

ClientGameState::~ClientGameState(){
	if(replay.isRecording()) replay.stopRecording();
	for(e_it i = entities.begin(), j = entities.end(); i != j; ++i){
		if(*i){
			delete *i;
		}
	}
	for(et_it i = entity_types.begin(), j = entity_types.end(); i != j; ++i){
		delete i->second;
	}
}
