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
#ifndef NRG_STATE_H
#define NRG_STATE_H
#include "nrg_core.h"
#include "nrg_connection.h"
#include "nrg_packet.h"

namespace nrg {

enum StateResult : uint32_t {
	STATE_CONTINUE = 0x00,
	STATE_EXIT_BIT = 0x10,
	STATE_FAILURE  = STATE_EXIT_BIT,
	STATE_CHANGE   = STATE_EXIT_BIT | 0x01
};

enum StateFlags : uint32_t {
	SFLAG_NONE      = 0x00,
	SFLAG_TIMED_OUT = 0x01
};

class Client;
class Server;
class Player;

struct StateConnectionOut {
	virtual bool ready() = 0;
	virtual bool enqueuePacket(Packet& p, PacketFlags f = PKTFLAG_NONE) = 0;
	virtual void resendLastPacket() = 0;
};

struct State {
	virtual bool init(Client* c, Server* s, Player* p) = 0;
	virtual bool onRecvPacket(Packet& p, PacketFlags f) = 0;
	virtual bool needsUpdate() const = 0;
	virtual size_t getTimeoutSeconds() const { return 10; }
	virtual StateResult update(StateConnectionOut& out, StateFlags f = SFLAG_NONE) = 0;
	virtual ~State(){}
};

struct StateConnectionOutImpl : StateConnectionOut {
	StateConnectionOutImpl(ConnectionOut& out);
	bool ready();
	bool enqueuePacket(Packet& p, PacketFlags f = PKTFLAG_NONE);
	void resendLastPacket();
	Status sendAllPackets();
	void update();
private:
	ConnectionOut& out;
	bool isready, resend;
	std::vector<std::pair<Packet, PacketFlags>> packet_queue;
};


}

#endif
