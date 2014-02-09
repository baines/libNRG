#include "nrg_server_state.h"
#include "nrg_input.h"
#include "nrg_os.h"
#include "nrg_varint.h"
#include <climits>
#include <iostream>

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
	std::cout << p.remaining() << std::endl;
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

StateResult ServerHandshakeState::update(ConnectionOut& out, StateFlags f){
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

ServerPlayerGameState::ServerPlayerGameState(const Snapshot& master, 
const DeltaSnapshotBuffer& dsb, Input& i, Player& p, int& l) 
: snapshot()
, master_ss(master)
, snaps(dsb)
, no_ack(true)
, ping(l)
, got_packet(true)
, ack_time(0)
, c_time(0)
, buffer()
, input(i)
, player(p) {

}

static const size_t NRG_MIN_SPGS_PACKET_LEN = 4;

bool ServerPlayerGameState::onRecvPacket(Packet& p, PacketFlags f){
	if(p.remaining() < NRG_MIN_SPGS_PACKET_LEN) return false;
	if(f & PKTFLAG_OUT_OF_ORDER) return true;

	got_packet = true;
	no_ack = false;

	p.read16(ack_time);
	TVarint<uint16_t> v;
	v.decode(p);
	c_time = ack_time + v.get();

	if(!input.readFromPacket(p)) return false;
	input.onUpdateNRG(player);

	return true;
}

bool ServerPlayerGameState::needsUpdate() const {
	return snapshot.getID() != master_ss.getID() && got_packet;
}

StateResult ServerPlayerGameState::update(ConnectionOut& out, StateFlags f){
	if(f & SFLAG_TIMED_OUT){
		return STATE_FAILURE;
	}
	
	got_packet = false;
	
	ping = std::max<int>(0, ((os::microseconds() / 1000) & USHRT_MAX) - c_time);

	if(no_ack){
		buffer.reset().write16(master_ss.getID());
		UVarint(ping).encode(buffer);
		master_ss.writeToPacket(buffer);
		out.sendPacket(buffer);
	} else {
		auto ss_r = find_if(snaps.rbegin(), snaps.rend(), [&](const DeltaSnapshot& s){
			return ack_time == s.getID();
		});
		if(ss_r != snaps.rend()){
			auto ss = ss_r.base();
			snapshot.reset();
			while(ss != snaps.end()){ snapshot.mergeWithNext(*ss); ++ss; }
			buffer.reset().write16(snapshot.getID());
			UVarint(ping).encode(buffer);
			snapshot.writeToPacket(buffer);
			out.sendPacket(buffer);
		} else {
			return STATE_FAILURE;
		}
	}
	return STATE_CONTINUE;
}
