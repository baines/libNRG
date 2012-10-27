#ifndef NRG_CONNECTION_H
#define NRG_CONNECTION_H
#include "nrg_core.h"
#include "nrg_netaddress.h"
#include "nrg_packet.h"
#include "nrg_socket.h"

namespace nrg {

enum ConnectionFlags {
	PKTFLAG_CONTINUATION = 0x01,
	PKTFLAG_CONTINUED    = 0x02,
};

class NRG_LIB ConnectionBase {
public:
	ConnectionBase(const NetAddress& remote_addr);
	size_t getHeaderSize() const { return 4; }
protected:
	uint16_t getPacketSeqNum(Packet& p) const;
	uint8_t getPacketFlags(Packet& p) const;
	NetAddress remote_addr;
	uint16_t seq_num;
};

class NRG_LIB ConnectionIncoming : protected ConnectionBase {
public:
	ConnectionIncoming(const NetAddress& remote_addr);
	bool addPacket(Packet& p);
	const NetAddress& getAddress() const;
	bool hasNewPacket() const;
	void getLatestPacket(Packet& p);
	int getIdleSeconds();
protected:
	bool isValidPacketHeader(uint16_t seq, uint8_t flags);
	Packet latest;
	PartialPacket partial;
};

class NRG_LIB ConnectionOutgoing : protected ConnectionBase {
public:
	ConnectionOutgoing(const NetAddress& remote_addr, const Socket& sock_out);
	void sendPacket(const Packet& p);
protected:
	const Socket& sock;
};

};

#endif
