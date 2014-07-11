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
#include "nrg_server_state.h"
#include "nrg_input.h"
#include "nrg_os.h"
#include "nrg_varint.h"
#include "nrg_server.h"
#include <climits>
#include <cassert>

using namespace nrg;
using namespace std;

namespace {
	enum {
		HS_NOT_STARTED = -1,
		HS_WAITING     = 0,
		HS_CLIENT_SYN  = 1,
		HS_ACCEPTED    = 1	
	};
}

ServerHandshakeState::ServerHandshakeState() 
: send_response(false){

}

bool ServerHandshakeState::onRecvPacket(Packet& p, PacketFlags f){
	if(p.remaining() == 1){
		uint8_t v;
		p.read8(v);
		if(v == HS_CLIENT_SYN) send_response = true;
		return true;
	} else {
		return false;	
	}
}

bool ServerHandshakeState::needsUpdate() const {
	return send_response;
}

StateResult ServerHandshakeState::update(StateConnectionOut& out, StateFlags f){
	if(f & SFLAG_TIMED_OUT){
		return STATE_FAILURE;
	} else {
		Packet p(1);
		p.write8(HS_ACCEPTED);
		out.sendPacket(p, PKTFLAG_STATE_CHANGE);
		send_response = false;
		return STATE_CHANGE;
	}
}

ServerPlayerGameState::ServerPlayerGameState()
: snapshot()
, no_ack(true)
, unackd_updates(0)
, seq(0)
, last_sent_id(0)
, ack_time(0)
, c_time(0)
, buffer()
, msg_manager()
, server(nullptr)
, player(nullptr) {

}

bool ServerPlayerGameState::init(Client* c, Server* s, Player* p){
	assert(s != nullptr);
	assert(p != nullptr);
	
	server = s;
	player = p;
	
	return true;
}

static const size_t NRG_MIN_SPGS_PACKET_LEN = 3;

bool ServerPlayerGameState::onRecvPacket(Packet& p, PacketFlags f){
	if(p.remaining() < NRG_MIN_SPGS_PACKET_LEN) return false;
	if(f & PKTFLAG_OUT_OF_ORDER) return true;

	no_ack = false;

	p.read16(ack_time);
	TVarint<uint16_t> v;
	v.decode(p);
	c_time = ack_time + v.get();

	InputBase* i = server->getInput();
	if(i != nullptr){
		if(!i->readFromPacket(p)) return false;
		i->onUpdate(*player);
	}
	
	unackd_updates = 0;
	
	return msg_manager.readFromPacket(p, os::milliseconds());
}

bool ServerPlayerGameState::needsUpdate() const {
	return snapshot.getID() != server->getSnapshot().getID() && unackd_updates < 3;
}

StateResult ServerPlayerGameState::update(StateConnectionOut& out, StateFlags f){
	if(f & SFLAG_TIMED_OUT){
		return STATE_FAILURE;
	}
	
	unackd_updates++;
	
	int ping = std::max<int>(0, (os::milliseconds() & USHRT_MAX) - c_time);

	if(no_ack){
		const Snapshot& master = server->getSnapshot();
		last_sent_id = master.getID();
		
		buffer.reset().write16(master.getID()).write8(seq++);
		UVarint(ping).encode(buffer);
		master.writeToPacket(buffer);
		msg_manager.writeToPacket(buffer, os::milliseconds());
		
		out.sendPacket(buffer);
	} else {
		const DeltaSnapshotBuffer& snaps = server->getDeltaSnapshots();
		
		auto ss_r = find_if(snaps.rbegin(), snaps.rend(), [&](const DeltaSnapshot& s){
			return ack_time == s.getID();
		});
		
		if(ss_r != snaps.rend()){
			snapshot.reset();

			for(auto ss = ss_r.base(); ss != snaps.end(); ++ss){
				snapshot.mergeWithNext(*ss);
			}
			
			auto last_it = find_if(snaps.rbegin(), snaps.rend(), [&](const DeltaSnapshot& s){
				return last_sent_id == s.getID();
			});
			
			seq += distance(last_it.base(), snaps.end());
			last_sent_id = snapshot.getID();
			
			buffer.reset().write16(snapshot.getID()).write8(seq);
			UVarint(ping).encode(buffer);
			snapshot.writeToPacket(buffer);
			msg_manager.writeToPacket(buffer, os::milliseconds());
			
			out.sendPacket(buffer);
		} else {
			return STATE_FAILURE;
		}
	}
	return STATE_CONTINUE;
}

void ServerPlayerGameState::registerMessageHandler(MessageBase&& m){
	msg_manager.addHandler(move(m));
}

void ServerPlayerGameState::registerMessageHandler(const MessageBase& m){
	msg_manager.addHandler(m);
}

void ServerPlayerGameState::sendMessage(const MessageBase& m){
	msg_manager.addMessage(m, os::milliseconds());
}

