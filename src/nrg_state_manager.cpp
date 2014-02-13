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

StateResult TransitionState::update(ConnectionOut& out, StateFlags f){
	StateResult r = STATE_FAILURE;
	
	if(client_mode){
		if(starting){
			out.sendPacket(buffer, PKTFLAG_STATE_CHANGE_ACK);
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
			out.resendLastPacket();
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
StateManager::StateManager(Server* s)
: StateManager(nullptr, s, nullptr){

}

StateManager::StateManager(Player* p)
: StateManager(nullptr, nullptr, p){

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

bool StateManager::update(ConnectionOut& out){
	if(states.empty()){
		return false;
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

		if(r == STATE_CHANGE && states.size() >= 2){
			if(state_ptr == &transition){
				states.pop_back();
				state_ptr = states.back();
			} else {
				state_ptr = &transition;
				transition.pre_init(states.back(), states[states.size()-2]);
				transition.init(c, s, p);
			}
		} else if(r & STATE_EXIT_BIT){
			states.clear();
			state_ptr = nullptr;
			return false;	
		}
	}
	
	return true;
}
