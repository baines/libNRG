#ifndef NRG_CONNECTION_H
#define NRG_CONNECTION_H
#include "nrg_core.h"
#include "nrg_netaddress.h"
#include "nrg_packet.h"
#include "nrg_socket.h"

namespace nrg {

enum PacketFlags : uint8_t {
	PKTFLAG_NONE             = 0x00,
	
	/* Only sent over the wire, not present in getLatestPacket() return value */
	PKTFLAG_CONTINUATION     = 0x01,
	PKTFLAG_CONTINUED        = 0x02,
	
	/* Both sent over the wire and present in getLatestPacket() return value */
	PKTFLAG_FINISHED         = 0x04,
	PKTFLAG_RETRANSMISSION   = 0x08,
	PKTFLAG_STATE_CHANGE     = 0x10,
	PKTFLAG_STATE_CHANGE_ACK = 0x20,
	
	/* Not sent over the wire, but used in getLastestPacket() return value */
	PKTFLAG_OUT_OF_ORDER     = 0x01
};

struct NRG_LIB ConnectionCommon {
public:
	ConnectionCommon(const NetAddress& remote_addr);
	size_t getHeaderSize() const { return sizeof(uint16_t) + sizeof(uint8_t); }
	void setTransform(PacketTransformation* transform);
	const NetAddress& remote_addr;
	uint16_t seq_num;
	PacketTransformation* transform;
};

class NRG_LIB ConnectionIn {
public:
	ConnectionIn(const NetAddress& remote_addr);
	bool addPacket(Packet& p);
	const NetAddress& getAddress() const;
	bool hasNewPacket() const;
	PacketFlags getLatestPacket(Packet& p);
	void setTransform(PacketTransformation* transform);
protected:
	ConnectionCommon cc;
	bool isValidPacketHeader(uint16_t seq, uint8_t flags);
	bool new_packet, first_packet, full_packet;
	PacketFlags latest_flags;
	Packet latest, buffer;
};

class NRG_LIB ConnectionOut {
public:
	ConnectionOut(const NetAddress& remote_addr, const Socket& sock_out);
	void sendPacket(Packet& p, PacketFlags f = PKTFLAG_NONE);
	void sendDisconnect(Packet& extra_data);
	void setTransform(PacketTransformation* transform);
	bool resendLastPacket(void);
protected:
	ConnectionCommon cc;
	const Socket& sock;
	Packet buffer, buffer2;
};

struct NRG_LIB Connection {
	Connection(const NetAddress& remote_addr, const Socket& sock_out)
		: in(remote_addr), out(remote_addr, sock_out){}
	ConnectionIn in;
	ConnectionOut out;
};

}

#endif
