#ifndef NRG_STATE_H
#define NRG_STATE_H
#include "nrg_core.h"
#include "nrg_connection.h"
#include "nrg_packet.h"
#include "nrg_snapshot.h"
#include "nrg_replay.h"
#include <map>
#include <vector>

namespace nrg {

typedef enum {
	STATE_EXIT_FAILURE = -1,
	STATE_EXIT_SUCCESS = 0,
	STATE_CONTINUE = 1
} StateUpdateResult;

struct NRG_LIB State {
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
class Input;

class NRG_LIB ClientGameState : public State {
public:
	ClientGameState(EventQueue& eq, const Socket& s, Input& i);
	bool addIncomingPacket(Packet& p);
	bool needsUpdate() const;
	StateUpdateResult update(ConnectionOutgoing& out);
	~ClientGameState();

	void registerEntity(Entity* e);
	double getSnapshotTiming() const;
	const ClientStats& getClientStats() const;

	void startRecordingReplay(const char* filename);
	void stopRecordingReplay();
private:
	std::vector<Entity*> entities;
	std::map<uint16_t, Entity*> entity_types;
	EventQueue& client_eventq;
	ClientStats* stats;
	int state_id;
	double ss_timer;
	uint32_t s_time_ms, c_time0_ms, c_time_ms;
	ClientSnapshot snapshot;
	Packet buffer;
	const Socket& sock;
	Input& input;
	ReplayRecorder replay;
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
	ServerPlayerGameState(const Snapshot& master_ss, 
		const DeltaSnapshotBuffer& dsb, Input& i, Player& p, int& latency);
	bool addIncomingPacket(Packet& p);
	bool needsUpdate() const;
	StateUpdateResult update(ConnectionOutgoing& out);
private:
	DeltaSnapshot snapshot;
	const Snapshot& master_ss;
	const DeltaSnapshotBuffer& snaps;
	int ackd_id;
	int& ping;
	uint32_t c_time;
	Packet buffer;
	Input& input;
	Player& player;
};

};

#endif
