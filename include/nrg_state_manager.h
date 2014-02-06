#ifndef NRG_STATE_MANAGER
#define NRG_STATE_MANAGER
#include "nrg_core.h"
#include "nrg_state.h"
#include <type_traits>
#include <vector>
#include <chrono>
#include <iostream>

namespace nrg {

using std::vector;
using namespace std::chrono;

struct NRG_LIB TransitionState : public State {
	void pre_init(State* old_s, State* new_s);
	bool init(Client* c, Server* s, Player* p);
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

class NRG_LIB StateManager {
public:
	StateManager(Client* c);
	StateManager(Server* s);
	StateManager(Player* p);
	void addState(State& s);
	bool onRecvPacket(Packet& packet, PacketFlags f);
	bool update(ConnectionOut& out);
private:
	vector<State*> states;
	State* state_ptr;
	TransitionState transition;
	time_point<system_clock> last_update;
	Client* c;
	Server* s;
	Player* p;
};

}

#endif
