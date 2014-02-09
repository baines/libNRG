#ifndef NRG_CLIENT_STATE_H
#define NRG_CLIENT_STATE_H
#include "nrg_core.h"
#include "nrg_state.h"
#include "nrg_replay.h"
#include "nrg_snapshot.h"
#include "nrg_message.h"
#include <map>
#include <vector>

namespace nrg {

class NRG_LIB ClientHandshakeState : public State {
public:
	ClientHandshakeState();
	bool init(Client*, Server*, Player*){ return true; }
	bool onRecvPacket(Packet& p, PacketFlags f);
	bool needsUpdate() const;
	size_t getTimeoutSeconds() const { return 2; }
	StateResult update(ConnectionOut& out, StateFlags f);
	~ClientHandshakeState();
private:
	Packet buffer;
	int phase, timeouts;
};

class Entity;
class Input;

class NRG_LIB ClientGameState : public State {
public:
	ClientGameState(EventQueue& eq, const Socket& s, Input& i);
	bool init(Client*, Server*, Player*){ return true; }
	bool onRecvPacket(Packet& p, PacketFlags f);
	bool needsUpdate() const;
	size_t getTimeoutSeconds() const { return 1; }
	StateResult update(ConnectionOut& out, StateFlags f);
	~ClientGameState();

	void registerEntity(Entity* e);
	void registerMessage(const MessageBase& m);
	
	double getSnapshotTiming() const;
	const ClientStats& getClientStats() const;

	void startRecordingReplay(const char* filename);
	void stopRecordingReplay();
private:
	std::vector<Entity*> entities;
	std::map<uint16_t, Entity*> entity_types;
	std::map<uint16_t, MessageBase*> messages;
	EventQueue& client_eventq;
	ClientStats* stats;
	int state_id, timeouts;
	double ss_timer;
	uint16_t s_time_ms;
	uint32_t c_time0_ms, c_time_ms;
	ClientSnapshot snapshot;
	Packet buffer;
	const Socket& sock;
	Input& input;
	ReplayRecorder replay;
	bool got_packet;
};

}

#endif
