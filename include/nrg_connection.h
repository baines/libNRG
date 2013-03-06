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
	PKTFLAG_FINISHED     = 0x04
};

class NRG_LIB ConnectionBase {
public:
	ConnectionBase(const NetAddress& remote_addr);
	size_t getHeaderSize() const { return 3; }
	void setTransform(PacketTransformation* transform);
protected:
	const NetAddress& remote_addr;
	uint16_t seq_num;
	PacketTransformation* transform;
};

class NRG_LIB ConnectionIncoming : public ConnectionBase {
public:
	ConnectionIncoming(const NetAddress& remote_addr);
	bool addPacket(Packet& p);
	const NetAddress& getAddress() const;
	bool hasNewPacket() const;
	void getLatestPacket(Packet& p);
	bool isLatestPacketFinal() const;
protected:
	bool isValidPacketHeader(uint16_t seq, uint8_t flags) const;
	bool new_packet, first_packet, full_packet, final_packet;
	Packet latest, buffer;
};

class NRG_LIB ConnectionOutgoing : public ConnectionBase {
public:
	ConnectionOutgoing(const NetAddress& remote_addr, const Socket& sock_out);
	void sendPacket(Packet& p);
	void sendDisconnect(Packet& extra_data);
protected:
	const Socket& sock;
	Packet buffer, buffer2;
};

};

#endif
