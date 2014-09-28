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
/** @file
 *  Functionality related to managing changing States
 */
#ifndef NRG_STATE_MANAGER
#define NRG_STATE_MANAGER
#include "nrg_core.h"
#include "nrg_state.h"
#include <type_traits>
#include <vector>

namespace nrg {

/** @cond INTERNAL_USE_ONLY */
/** Class to reliably handle switching between two states on both server and client */
struct TransitionState : public State {
	void pre_init(State* old_s, State* new_s);
	bool init(Client*, Server*, Player*);
	bool onRecvPacket(Packet& p, PacketFlags f);
	bool needsUpdate() const;
	StateResult update(StateConnectionOut& out, StateFlags f);
	size_t getTimeoutSeconds() const;
private:
	int timeouts;
	bool client_mode, starting, resend, done;
	State *old_state, *new_state;
	Packet buffer;
};

/** Class to manage the changing and transision of States */
class StateManager {
public:
	/** Constructor, either server or client-side */
	StateManager(Client* c, Server* s, Player* p);
	
	/** Constructor, server-side */
	StateManager(Server* s, Player* p);
	
	/** Constructor, client-side */
	StateManager(Client* c);
	
	/** Adds a state to be managed */
	void addState(State& s);
	
	/** Called by the server or client to pass along a packet to the current State */
	bool onRecvPacket(Packet& packet, PacketFlags f);
	
	/** Called by the server or client to update the current State */
	Status update(StateConnectionOut& out);
private:
	std::vector<State*> states;
	State* state_ptr;
	TransitionState transition;
	uint32_t last_update;
	Client* c;
	Server* s;
	Player* p;
};

}

#endif
