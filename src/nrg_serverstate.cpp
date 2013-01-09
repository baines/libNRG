#include "nrg_state.h"

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

ServerPlayerGameState::ServerPlayerGameState(const Snapshot& master) 
: snapshot(), master(master){

}

bool ServerPlayerGameState::addIncomingPacket(Packet& p){
	return false;
}

bool ServerPlayerGameState::needsUpdate() const {
	return snapshot.getID() != master.getID();
}

StateUpdateResult ServerPlayerGameState::update(ConnectionOutgoing& out){
	if(snapshot.merge(master)){
		Packet p;
		p.write16(snapshot.getID());
		p.write16(0); // TODO: Input ID
		snapshot.writeToPacket(p);
		out.sendPacket(p);	
	} else {
		// kick
	}
	return STATE_CONTINUE;
}
