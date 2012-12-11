#include "nrg_state.h"

using namespace nrg;

//TODO

ServerHandshakeState::ServerHandshakeState(){};

bool ServerHandshakeState::addIncomingPacket(Packet& p){
	return false;
}

bool ServerHandshakeState::needsUpdate() const {
	return false;
}

StateUpdateResult ServerHandshakeState::update(ConnectionOutgoing& out){
	return STATE_CONTINUE;
}

ServerPlayerGameState::ServerPlayerGameState(const Snapshot& master) 
: snapshot(), master(master){

}

bool ServerPlayerGameState::addIncomingPacket(Packet& p){
	return false;
}

bool ServerPlayerGameState::needsUpdate() const {
	return false;
}

StateUpdateResult ServerPlayerGameState::update(ConnectionOutgoing& out){
	return STATE_CONTINUE;
}
