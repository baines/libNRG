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

struct State {
	virtual bool init(Client* c, Server* s, Player* p) = 0;
	virtual bool onRecvPacket(Packet& p, PacketFlags f) = 0;
	virtual bool needsUpdate() const = 0;
	virtual size_t getTimeoutSeconds() const { return 10; }
	virtual StateResult update(ConnectionOut& out, StateFlags f = SFLAG_NONE) = 0;
	virtual ~State(){}
};

}

#endif
