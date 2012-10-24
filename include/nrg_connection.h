#ifndef NRG_CONNECTION_H
#define NRG_CONNECTION_H
#include "nrg_core.h"
#include "nrg_netaddress.h"
#include "nrg_packet.h"

namespace nrg {

class NRG_LIB Connection {
public:
	Connection(const NetAddress& remote_addr);
	bool addIncomingPacket(const Packet& p);
	const NetAddress& getAddress() const;
	bool hasNewPacket() const;
	void getLatestPacket(Packet& p);
	int getIdleSeconds();
protected:
	Packet latest;
	NetAddress addr;
};

};

#endif
