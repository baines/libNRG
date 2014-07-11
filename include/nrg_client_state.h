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
	StateResult update(StateConnectionOut& out, StateFlags f);
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
	StateResult update(StateConnectionOut& out, StateFlags f);
	~ClientGameState();

	void registerEntity(Entity* e);
	void registerMessage(const MessageBase& m);
	void registerMessage(MessageBase&& m);
	
	void sendMessage(const MessageBase& m);
	
	double getInterpTimer() const;
	const ClientStats& getClientStats() const;

	void startRecordingReplay(const char* filename);
	void stopRecordingReplay();
private:
	std::function<Entity*(ClientSnapshot::Action, uint16_t, uint16_t)> snap_func;
	Entity* SnapFuncImpl(ClientSnapshot::Action, uint16_t, uint16_t);
	std::map<uint16_t, Entity*> entities;
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
