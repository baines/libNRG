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
#ifndef NRG_SERVER_STATE_H
#define NRG_SERVER_STATE_H
#include "nrg_core.h"
#include "nrg_state.h"
#include "nrg_snapshot.h"
#include "nrg_message.h"
#include "nrg_message_manager.h"

namespace nrg {

struct ServerHandshakeState : public State {
	ServerHandshakeState();
	bool init(Client*, Server*, Player*);
	bool onRecvPacket(Packet& p, PacketFlags f);
	bool needsUpdate() const { return true; }
	StateResult update(StateConnectionOut& out, StateFlags f);
private:
	Server* server;
	Player* player;
	int8_t response;
	Packet packet;
};

class ServerPlayerGameState : public State {
public:
	ServerPlayerGameState();
	bool init(Client*, Server*, Player*);
	bool onRecvPacket(Packet& p, PacketFlags f);
	bool needsUpdate() const;
	StateResult update(StateConnectionOut& out, StateFlags f);
	void registerMessageHandler(MessageBase&& m);
	void registerMessageHandler(const MessageBase& m);
	void sendMessage(const MessageBase& m);
private:
	DeltaSnapshot snapshot;
	bool no_ack;
	int unackd_updates;
	uint8_t seq;
	uint16_t last_sent_id;
	uint16_t ack_time;
	uint16_t c_time;
	Packet buffer;
	MessageManager msg_manager;
	Server* server;
	Player* player;
};

}

#endif
