#ifndef NRG_SERVER_STATE_H
#define NRG_SERVER_STATE_H
#include "nrg_core.h"
#include "nrg_state.h"
#include "nrg_snapshot.h"
#include "nrg_message.h"

namespace nrg {

class Input;

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
	ServerPlayerGameState(const Snapshot& master_ss, 
		const DeltaSnapshotBuffer& dsb, Input& i, Player& p, int& latency);
	bool init(Client*, Server*, Player*){ return true; }
	bool onRecvPacket(Packet& p, PacketFlags f);
	bool needsUpdate() const;
	StateResult update(ConnectionOut& out, StateFlags f);
private:
	DeltaSnapshot snapshot;
	const Snapshot& master_ss;
	const DeltaSnapshotBuffer& snaps;
	bool no_ack;
	int& ping;
	bool got_packet;
	uint16_t ack_time;
	uint16_t c_time;
	Packet buffer;
	Input& input;
	Player& player;
};

}

#endif
