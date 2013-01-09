#ifndef NRG_CLIENT_H
#define NRG_CLIENT_H
#include "nrg_core.h"
#include "nrg_socket.h"
#include "nrg_netaddress.h"
#include "nrg_state.h"
#include <vector>

namespace nrg {

class NRG_LIB Client {
public:
	Client();
	Client(const NetAddress& server_addr);
	~Client();
	status_t connect(const NetAddress& server_addr);
	bool isConnected() const;
	const NetAddress& getAddress() const;
	//Input& getInput() const;
	
	status_t update();
protected:
	UDPSocket sock;
	Packet buffer;
	NetAddress serv_addr;
	ConnectionIncoming in;
	ConnectionOutgoing out;
	std::vector<State*> states;
	ClientHandshakeState handshake;
	ClientGameState game_state;
};

};

#endif
