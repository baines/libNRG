#include "nrg_player_impl.h"

using namespace nrg;

PlayerImpl::PlayerImpl(uint16_t id, Server& s, const NetAddress& addr) 
: server(s)
, addr(addr)
, sock(s.getSocket())
, con(addr, sock)
, buffer(NRG_MAX_PACKET_SIZE)
, ping(0)
, state_manager(nullptr, &s, this)
, handshake()
, game_state()
, id(id)
, connected(true) {
	state_manager.addState(game_state);
	state_manager.addState(handshake);
}

bool PlayerImpl::addPacket(Packet& p){
	bool valid = false;
	if(con.in.addPacket(p)){
		if(con.in.hasNewPacket()){
			PacketFlags f = con.in.getLatestPacket(buffer.reset());
			if(f & PKTFLAG_FINISHED){
				valid = true;				
				connected = false;
			} else {
				valid = state_manager.onRecvPacket(buffer, f);
			}
		} else {
			valid = true;
		}
	}
	return valid;
}

bool PlayerImpl::update(){
	return state_manager.update(con.out);
}

void PlayerImpl::kick(const char* reason){
	buffer.reset().writeArray((uint8_t*)reason, strlen(reason));
	con.out.sendDisconnect(buffer);
	connected = false;
}

void PlayerImpl::registerMessageHandler(MessageBase&& m){
	game_state.registerMessageHandler(std::move(m));
}

void PlayerImpl::registerMessageHandler(const MessageBase& m){
	game_state.registerMessageHandler(m);
}

void PlayerImpl::sendMessage(const MessageBase& m){
	game_state.sendMessage(m);
}

bool PlayerImpl::isConnected() const {
	return connected;
}

int PlayerImpl::getPing() const {
	return ping;
}
