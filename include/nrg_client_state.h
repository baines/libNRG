#ifndef NRG_CLIENT_STATE_H
#define NRG_CLIENT_STATE_H
#include "nrg_core.h"
#include "nrg_state.h"
#include "nrg_replay.h"
#include "nrg_snapshot.h"
#include "nrg_message.h"
#include "nrg_message_manager.h"
#include <map>
#include <vector>
#include <functional>

namespace nrg {

class ClientHandshakeState : public State {
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
class InputBase;

class ClientGameState : public State, public EntityManager {
public:
	ClientGameState();
	bool init(Client*, Server*, Player*);
	bool onRecvPacket(Packet& p, PacketFlags f);
	bool needsUpdate() const;
	size_t getTimeoutSeconds() const { return 1; }
	StateResult update(ConnectionOut& out, StateFlags f);
	~ClientGameState();

	void registerEntity(Entity* e);
	void registerMessage(const MessageBase& m);
	void registerMessage(MessageBase&& m);
	
	void sendMessage(const MessageBase& m);
	
	float getInterpTimer() const;
	const ClientStats& getClientStats() const;

	void startRecordingReplay(const char* filename);
	void stopRecordingReplay();
private:
	std::function<Entity*(ClientSnapshot::Action, uint16_t, uint16_t)> snap_func;
	Entity* SnapFuncImpl(ClientSnapshot::Action, uint16_t, uint16_t);
	std::vector<Entity*> entities;
	std::map<uint16_t, Entity*> entity_types;
	MessageManager msg_manager;
	EventQueue* client_eventq;
	std::unique_ptr<ClientStats> stats;
	int timeouts;
	double ss_timer;
	uint8_t server_seq_prev;
	uint16_t server_ms_prev;
	uint32_t client_ms, client_ms_prev, interval;
	ClientSnapshot snapshot;
	Packet buffer;
	InputBase* input;
	ReplayRecorder replay;
	bool got_packet;
	Client* client;
};

}

#endif
