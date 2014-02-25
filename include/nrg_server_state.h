#ifndef NRG_SERVER_STATE_H
#define NRG_SERVER_STATE_H
#include "nrg_core.h"
#include "nrg_state.h"
#include "nrg_snapshot.h"
#include "nrg_message.h"

namespace nrg {

struct NRG_LIB ServerHandshakeState : public State {
	ServerHandshakeState();
	bool init(Client*, Server*, Player*){ return true; }
	bool onRecvPacket(Packet& p, PacketFlags f);
	bool needsUpdate() const;
	StateResult update(ConnectionOut& out, StateFlags f);
private:
	bool send_response;
};

class NRG_LIB ServerPlayerGameState : public State {
public:
	ServerPlayerGameState();
	bool init(Client*, Server*, Player*);
	bool onRecvPacket(Packet& p, PacketFlags f);
	bool needsUpdate() const;
	StateResult update(ConnectionOut& out, StateFlags f);
private:
	DeltaSnapshot snapshot;
	bool no_ack;
	bool got_packet;
	uint8_t seq;
	uint16_t ack_time;
	uint16_t c_time;
	Packet buffer;
	Server* server;
	Player* player;
};

}

#endif
