#ifndef NRG_STATE_H
#define NRG_STATE_H
#include "nrg_core.h"
#include "nrg_connection.h"
#include "nrg_packet.h"
#include "nrg_snapshot.h"
#include <map>
#include <vector>

namespace nrg {

typedef enum {
	STATE_EXIT_FAILURE = -1,
	STATE_EXIT_SUCCESS = 0,
	STATE_CONTINUE = 1
} StateUpdateResult;

class NRG_LIB State {
public:
	State(){};
	virtual bool addIncomingPacket(Packet& p) = 0;
	virtual bool needsUpdate() const = 0;
	virtual size_t getTimeoutSeconds() const { return 10; }
	virtual StateUpdateResult update(ConnectionOutgoing& out) = 0;
	virtual ~State(){};
};

class NRG_LIB ClientHandshakeState : public State {
public:
	ClientHandshakeState();
	bool addIncomingPacket(Packet& p);
	bool needsUpdate() const;
	StateUpdateResult update(ConnectionOutgoing& out);
	~ClientHandshakeState();
private:
	enum HandShakePhase {
		NOT_STARTED = -1,
		WAITING_ON_RESPONSE = 0,
		ACCEPTED = 1,
		REJECTED_FULL = 2
	} phase;
};

class Entity;

class NRG_LIB ClientGameState : public State {
public:
	ClientGameState(EventQueue& eq, const Socket& s);
	bool addIncomingPacket(Packet& p);
	bool needsUpdate() const;
	StateUpdateResult update(ConnectionOutgoing& out);
	~ClientGameState();

	void registerEntity(Entity* e);
	double getSnapshotTiming() const;
private:
	std::vector<Entity*> entities, updated_entities;
	std::map<uint16_t, Entity*> entity_types;
	EventQueue& client_eventq;
	int state_id;
	uint32_t s_time_ms, c_time_ms;
	ClientSnapshot snapshot, next_snapshot;
	Packet buffer;
	const Socket& sock;
};

struct NRG_LIB ServerHandshakeState : public State {
	ServerHandshakeState();
	bool addIncomingPacket(Packet& p);
	bool needsUpdate() const;
	StateUpdateResult update(ConnectionOutgoing& out);
private:
	bool send_response;
};

class NRG_LIB ServerPlayerGameState : public State {
public:
	ServerPlayerGameState(const Snapshot& master_ss, const DeltaSnapshotBuffer& dsb);
	bool addIncomingPacket(Packet& p);
	bool needsUpdate() const;
	StateUpdateResult update(ConnectionOutgoing& out);
private:
	Snapshot snapshot;
	const Snapshot& master_ss;
	const DeltaSnapshotBuffer& snaps;
	int ackd_id;
	int latency;
	Packet buffer;
};

};

#endif
