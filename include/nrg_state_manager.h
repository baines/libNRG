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
#ifndef NRG_STATE_MANAGER
#define NRG_STATE_MANAGER
#include "nrg_core.h"
#include "nrg_state.h"
#include <type_traits>
#include <vector>

namespace nrg {

using std::vector;

struct  TransitionState : public State {
	void pre_init(State* old_s, State* new_s);
	bool init(Client*, Server*, Player*);
	bool onRecvPacket(Packet& p, PacketFlags f);
	bool needsUpdate() const;
	StateResult update(ConnectionOut& out, StateFlags f);
	size_t getTimeoutSeconds() const;
private:
	int timeouts;
	bool client_mode, starting, resend, done;
	State *old_state, *new_state;
	Packet buffer;
};

class  StateManager {
public:
	StateManager(Client* c, Server* s, Player* p);
	StateManager(Server* s, Player* p);
	StateManager(Client* c);
	void addState(State& s);
	bool onRecvPacket(Packet& packet, PacketFlags f);
	Status update(ConnectionOut& out);
private:
	vector<State*> states;
	State* state_ptr;
	TransitionState transition;
	uint32_t last_update;
	Client* c;
	Server* s;
	Player* p;
};

}

#endif
