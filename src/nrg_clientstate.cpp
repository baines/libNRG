/*
  LibNRG - Networking for Real-time Games
  
  Copyright (C) 2012-2014 Alex Baines <alex@abaines.me.uk>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
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

using namespace std::placeholders;

ClientGameState::ClientGameState() 
: snap_func(bind(&ClientGameState::SnapFuncImpl, this, _1, _2, _3))
, entities()
, entity_types()
, msg_manager()
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
	for(auto& pair : entities){
	    Entity* e = pair.second;
		e->markUpdated(false);
		for(FieldBase* f = e->getFirstField(); f; f = f->getNextField()){
			f->shiftData();			
			f->setUpdated(false);
		}
	}
	
	if(!snapshot.readFromPacket(p, snap_func)) return false;	
	if(!msg_manager.readFromPacket(p, server_ms)) return false;

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
	
	uint16_t diff_ms = now_ms - client_ms;
	
	TVarint<uint16_t>(diff_ms).encode(buffer);
	if(input) input->writeToPacket(buffer);
	msg_manager.writeToPacket(buffer, server_ms_prev + diff_ms);

	out.sendPacket(buffer);

	return STATE_CONTINUE;
}

Entity* ClientGameState::SnapFuncImpl(ClientSnapshot::Action a, uint16_t eid, uint16_t etype){
	Entity* ret = nullptr;
	
	switch(a){
		case ClientSnapshot::Action::Get: {
			auto it = entities.find(eid);
			if(it != entities.end()){
				ret = it->second;
			}
			break;
		}
		case ClientSnapshot::Action::Create: {
			auto i = entity_types.find(etype);
			if(i != entity_types.end()){
				Entity* e = i->second->clone();
				e->setID(eid);

				ret = entities.emplace(eid, e).first->second;

				e->onCreate(*client);
				client_eventq->pushEvent(EntityEvent{ ENTITY_CREATED, eid, etype, e });
			}
			break;
		}
		case ClientSnapshot::Action::Destroy: {
			auto it = entities.find(eid);
			if(it != entities.end()){
				Entity*& e = it->second;
				e->onDestroy(*client);
				client_eventq->pushEvent(EntityEvent{ ENTITY_DESTROYED, eid, e->getType(), e });
				delete e;
				entities.erase(it);
			}
			break;
		}
		case ClientSnapshot::Action::Update: {
			auto it = entities.find(eid);
			if(it != entities.end()){
				Entity*& e = it->second;
				e->onUpdate(*client);
				client_eventq->pushEvent(EntityEvent{ ENTITY_UPDATED, eid, e->getType(), e });
			}
			break;
		}
	}
	
	return ret;
}

void ClientGameState::registerEntity(Entity* e){
	e->setManager(this);
	entity_types.insert(make_pair(e->getType(), e));
}

void ClientGameState::registerMessage(const MessageBase& m){
	msg_manager.addHandler(m);
}

void ClientGameState::registerMessage(MessageBase&& m){
	msg_manager.addHandler(move(m));
}

void ClientGameState::sendMessage(const MessageBase& m){
	uint16_t diff_ms = os::milliseconds() - client_ms;
	msg_manager.addMessage(m, server_ms_prev + diff_ms);
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
	for(auto& pair : entities){
		delete pair.second;
	}
	for(auto& pair : entity_types){
		delete pair.second;
	}
}
