#include "nrg_server_state.h"
#include "nrg_input.h"
#include "nrg_os.h"
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

ServerHandshakeState::ServerHandshakeState() : send_response(false){}

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
const DeltaSnapshotBuffer& dsb, Input& i, Player& p, int& l) : snapshot(), 
master_ss(master), snaps(dsb), ackd_id(-1), ping(l), c_time(0), buffer(), input(i), player(p) {

}

static const size_t NRG_MIN_SPGS_PACKET_LEN = 6;

bool ServerPlayerGameState::onRecvPacket(Packet& p, PacketFlags f){
	if(p.remaining() < NRG_MIN_SPGS_PACKET_LEN) return false;
	uint16_t new_ackd_id = 0;

	p.read16(new_ackd_id).read32(c_time);
	ackd_id = new_ackd_id;

	if(!input.readFromPacket(p)) return false;
	input.onUpdateNRG(player);

	return true;
}

bool ServerPlayerGameState::needsUpdate() const {
	return snapshot.getID() != master_ss.getID();
}

StateResult ServerPlayerGameState::update(ConnectionOut& out, StateFlags f){
	if(f & SFLAG_TIMED_OUT){
		return STATE_FAILURE;
	}
	
	ping = std::max<int>(0, (os::microseconds() / 1000) - c_time);

	if(ackd_id == -1){
		buffer.reset().write16(master_ss.getID()).write32(os::microseconds()/1000)
			.write16(ping).write16(0);
		master_ss.writeToPacket(buffer);
		out.sendPacket(buffer);
	} else {
		const DeltaSnapshot* ss = snaps.find(ackd_id + 1);
		if(ss != NULL){
			snapshot.reset();
			for(uint16_t i = ackd_id + 1; i != snaps.getCurrentID(); ++i){
				snapshot.mergeWithNext(*snaps.find(i));
			}
			buffer.reset().write16(snapshot.getID()).write32(os::microseconds()/1000).
				write16(ping).write16(0);
			snapshot.writeToPacket(buffer);
			out.sendPacket(buffer);
		} else {
			return STATE_FAILURE;
		}
	}
	return STATE_CONTINUE;
}
