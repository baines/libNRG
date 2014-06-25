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

Status PlayerImpl::update(){
	if(!state_manager.update(con.out)){
		return Status("Client update failed.");
	} else {
		return con.out.getLastStatus();
	}
}

const NetAddress& PlayerImpl::getRemoteAddress() const {
	return addr;
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
