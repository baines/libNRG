#ifndef NRG_CONNECTION_H
#define NRG_CONNECTION_H
#include "nrg_core.h"
#include "nrg_netaddress.h"
#include "nrg_packet.h"
#include "nrg_socket.h"

namespace nrg {

class NRG_LIB ConnectionBase {
public:
	size_t getHeaderSize(){ return 4; }
protected:
	bool isValidPacket(const Packet& p);
	uint16_t getPacketSeqNum(const Packet& p) const;
	uint8_t getPacketFlags(const Packet& p) const;
};

class NRG_LIB ConnectionIncoming : protected ConnectionBase {
public:
	ConnectionIncoming(const NetAddress& remote_addr);
	bool addPacket(const Packet& p);
	const NetAddress& getAddress() const;
	bool hasNewPacket() const;
	void getLatestPacket(Packet& p);
	int getIdleSeconds();
protected:
	Packet latest;
	NetAddress addr;
};

class NRG_LIB ConnectionOutgoing : protected ConnectionBase {
public:
	ConnectionOutgoing(const NetAddress& remote_addr, const Socket& sock_out);
	void sendPacket(const Packet& p);
};

};

#endif
