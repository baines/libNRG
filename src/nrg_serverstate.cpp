#include "nrg_state.h"
#include "nrg_os.h"
#include <climits>

using namespace nrg;

//TODO

ServerHandshakeState::ServerHandshakeState() : send_response(false){};

bool ServerHandshakeState::addIncomingPacket(Packet& p){
	if(p.remaining() == 1){
		uint8_t v;
		p.read8(v);
		if(v == 1) send_response = true;
		return true;
	} else {
		return false;	
	}
}

bool ServerHandshakeState::needsUpdate() const {
	return send_response;
}

StateUpdateResult ServerHandshakeState::update(ConnectionOutgoing& out){
	Packet p(1);
	p.write8(1);
	out.sendPacket(p);
	send_response = false;
	return STATE_EXIT_SUCCESS;
}

ServerPlayerGameState::ServerPlayerGameState(const Snapshot& master, 
const DeltaSnapshotBuffer& dsb) : snapshot(), master_ss(master), snaps(dsb), 
ackd_id(-1), latency(0), buffer() {

}

bool ServerPlayerGameState::addIncomingPacket(Packet& p){
	if(p.remaining() != 6) return false;
	uint16_t new_ackd_id = 0;
	uint32_t c_time = 0;

	p.read16(new_ackd_id).read32(c_time);
	ackd_id = new_ackd_id;

	latency = (os::microseconds() / 1000) - c_time;
	printf("%p latency: %dms\n", this, latency);
	return true;
}

bool ServerPlayerGameState::needsUpdate() const {
	return snapshot.getID() != master_ss.getID();
}

StateUpdateResult ServerPlayerGameState::update(ConnectionOutgoing& out){
	//printf("%p latency: %dms\n", this, latency);
	if(ackd_id == -1){
		//TODO: input id
		buffer.reset().write16(master_ss.getID()).write32(os::microseconds()/1000).write16(0);
		master_ss.writeToPacket(buffer);
		out.sendPacket(buffer);
	} else {
		const DeltaSnapshot* ss = snaps.find((ackd_id + 1) & USHRT_MAX);
		if(ss != NULL){
			snapshot.reset();
			for(uint16_t i = ackd_id + 1; i != snaps.getCurrentID(); ++i){
				snapshot.mergeWithNext(*snaps.find(i));
			}
			buffer.reset().write16(snapshot.getID()).write32(os::microseconds()/1000).write16(0);
			snapshot.writeToPacket(buffer);
			out.sendPacket(buffer);
		} else {
			return STATE_EXIT_FAILURE;
		}
	}
	return STATE_CONTINUE;
}
