#ifndef NRG_SERVER_H
#define NRG_SERVER_H
#include "nrg_core.h"

class NRG_LIB Server {
public:
	Server();
	void update();
protected:
	UDPSocket sock;
	std::set<PlayerConnection> clients;

};

#endif
