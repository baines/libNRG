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
#ifndef NRG_PLAYER_IMPL_H
#define NRG_PLAYER_IMPL_H
#include "nrg_server.h"
#include "nrg_server_state.h"
#include "nrg_state_manager.h"
#include "nrg_message_manager.h"
#include "nrg_connection.h"

namespace nrg {

class PlayerImpl : public Player {
public:
	PlayerImpl(uint16_t id, Server& server, const NetAddress& addr);
	bool addPacket(Packet& p);
	Status update();
	void kick(const char* reason);
	const Server* getServer() const { return &server; }
	const NetAddress& getRemoteAddress() const;
	void registerMessageHandler(MessageBase&& m);
	void registerMessageHandler(const MessageBase& m);
	void sendMessage(const MessageBase& m);
	bool isConnected() const;
	int getPing() const;
	uint16_t getID() const { return id; }
protected:
	const Server& server;
	const NetAddress& addr;
	const UDPSocket& sock;
	Connection con;
	StateConnectionOutImpl state_con;
	Packet buffer;
	
	int ping;
	StateManager state_manager;
	ServerHandshakeState handshake;
	ServerPlayerGameState game_state;
	uint16_t id;
	bool connected;
};

}

#endif
