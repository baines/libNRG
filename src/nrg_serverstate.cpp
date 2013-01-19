#include "nrg_state.h"
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
ackd_id(-1), buffer() {

}

bool ServerPlayerGameState::addIncomingPacket(Packet& p){
	return false;
}

bool ServerPlayerGameState::needsUpdate() const {
	return snapshot.getID() != master_ss.getID();
}

StateUpdateResult ServerPlayerGameState::update(ConnectionOutgoing& out){
	if(ackd_id == -1){
		buffer.reset().write16(master_ss.getID()).write16(0); // TODO: Input ID
		master_ss.writeToPacket(buffer);
		out.sendPacket(buffer);
	} else {
		const DeltaSnapshot* ss = snaps.find((ackd_id + 1) & USHRT_MAX);
		if(ss != NULL){
			snapshot.reset();
			for(uint16_t i = ackd_id + 1; i != snaps.getCurrentID(); ++i){
				snapshot.mergeWithNext(*snaps.find(i));
			}
			buffer.reset().write16(snapshot.getID()).write16(0);
			snapshot.writeToPacket(buffer);
			out.sendPacket(buffer);
		} else {
			// kick
		}
	}
	return STATE_CONTINUE;
}
