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
#include "nrg_state_manager.h"
#include "nrg_os.h"

using namespace nrg;
using namespace std;

void TransitionState::pre_init(State* old_s, State* new_s){
	old_state = old_s;
	new_state = new_s;
}

bool TransitionState::init(Client* c, Server* s, Player* p){
	resend = false;
	done = false;
	starting = true;
	client_mode = c != nullptr;
	timeouts = 0;

	new_state->init(c, s, p);

	return true;
}

bool TransitionState::onRecvPacket(Packet& p, PacketFlags f){
	if(client_mode){
		new_state->onRecvPacket(p, f);
		done = true;
	} else {
		if(f & PKTFLAG_STATE_CHANGE_ACK){
			done = true;
		} else if(f & PKTFLAG_RETRANSMISSION){
			resend = true;
		}
	}
	return true;
}

bool TransitionState::needsUpdate() const {
	if(client_mode){
		return starting || done;
	} else {
		return resend || done;
	}
}

StateResult TransitionState::update(StateConnectionOut& out, StateFlags f){
	StateResult r = STATE_FAILURE;

	if(client_mode){
		if(starting){
			if(!out.sendPacket(buffer, PKTFLAG_STATE_CHANGE_ACK)) return STATE_CONTINUE;
			starting = false;
			r = STATE_CONTINUE;
		} else if(done){
			r = STATE_CHANGE;
		} else if(f & SFLAG_TIMED_OUT){
			if(++timeouts > 5){
				r = STATE_FAILURE;
			} else {
				out.resendLastPacket();
				r = STATE_CONTINUE;
			}
		}
	} else {
		if(resend){
			if(!out.resendLastPacket()) return STATE_CONTINUE;
			resend = false;
			r = STATE_CONTINUE;
		} else if(f & SFLAG_TIMED_OUT || !done){
			r = STATE_FAILURE;
		} else {
			r = STATE_CHANGE;
		}
	}

	return r;
}

size_t TransitionState::getTimeoutSeconds() const {
	return client_mode ? 1 : 10;
}

StateManager::StateManager(Client* c, Server* s, Player* p)
: state_ptr(nullptr)
, last_update(os::seconds())
, c(c)
, s(s)
, p(p){

}

StateManager::StateManager(Client* c)
: StateManager(c, nullptr, nullptr){

}
StateManager::StateManager(Server* s, Player* p)
: StateManager(nullptr, s, p){

}

void StateManager::addState(State& s){
	states.push_back(&s);
}

bool StateManager::onRecvPacket(Packet& packet, PacketFlags f){
	if(states.empty()){
		return false;
	} else if(!state_ptr){
		state_ptr = states.back();
		state_ptr->init(c, s, p);
		last_update = os::seconds();
	}
	return state_ptr->onRecvPacket(packet, f);
}

Status StateManager::update(StateConnectionOut& out){
	if(states.empty()){
		return Status("There are no registered states left.");
	} else if(!state_ptr){
		state_ptr = states.back();
		state_ptr->init(c, s, p);
		last_update = os::seconds();
	}

	std::underlying_type<StateFlags>::type f = SFLAG_NONE;

	size_t timeo = state_ptr->getTimeoutSeconds();
	if(timeo && (os::seconds() - last_update) > timeo){
		f |= SFLAG_TIMED_OUT;
	}

	if(state_ptr->needsUpdate() || (f & SFLAG_TIMED_OUT)){
		StateResult r = state_ptr->update(out, static_cast<StateFlags>(f));
		last_update = os::seconds();

		if(r.id == STATE_CHANGE && states.size() >= 2){
			if(state_ptr == &transition){
				states.pop_back();
				state_ptr = states.back();
			} else {
				state_ptr = &transition;
				transition.pre_init(states.back(), states[states.size()-2]);
				transition.init(c, s, p);
			}
		} else if(r.id & STATE_EXIT_BIT){
			states.clear();
			state_ptr = nullptr;
			return Status(r.msg ? r.msg : "State update failed.");
		}
	}

	return true;
}
