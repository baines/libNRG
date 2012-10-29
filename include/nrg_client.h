#ifndef NRG_CLIENT_H
#define NRG_CLIENT_H
#include "nrg_core.h"
#include <vector>

namespace nrg {

class NRG_LIB Client {
public:
	Client();
	Client(const NetAddress& server_addr);
	status_t connect(const NetAddress& server_addr);
	bool isConnected() const;
	const NetAddress& getAddress() const;
	//Input& getInput() const;
	
	bool sendUpdate();
	bool recvUpdate();
protected:
	UDPSocket sock;
	NetAddress serv_addr;
	ConnectionIncoming in;
	ConnectionOutgoing out;
	std::vector<State*> states;
};

};

#endif
