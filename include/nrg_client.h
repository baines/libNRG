#ifndef NRG_CLIENT_H
#define NRG_CLIENT_H
#include "nrg_core.h"
#include "nrg_socket.h"
#include "nrg_netaddress.h"
#include "nrg_state.h"
#include "nrg_event.h"
#include "nrg_input.h"
#include <vector>

namespace nrg {

class NRG_LIB Client {
public:
	Client(Input& input = null_input);
	Client(const NetAddress& server_addr, Input& input = null_input);
	~Client();
	status_t connect(const NetAddress& server_addr);
	bool isConnected() const;
	const NetAddress& getAddress() const;

	void registerEntity(Entity* e);
	status_t update();
	bool pollEvent(Event& e);
protected:
	UDPSocket sock;
	Input& input;
	Packet buffer;
	NetAddress serv_addr;
	ConnectionIncoming in;
	ConnectionOutgoing out;
	EventQueue eventq;
	std::vector<State*> states;
	ClientHandshakeState handshake;
	ClientGameState game_state;
	char dc_reason[NRG_MAX_ERRORMSG_LEN];
};

};

#endif
