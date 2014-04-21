#ifndef NRG_CONNECTION_H
#define NRG_CONNECTION_H
#include "nrg_core.h"
#include "nrg_netaddress.h"
#include "nrg_packet.h"
#include "nrg_socket.h"
#include <bitset>

namespace nrg {

enum PacketFlags : uint8_t {
	PKTFLAG_NONE             = 0x00,
	
	/* Not sent over the wire, but used in getLastestPacket() return value */
	PKTFLAG_OUT_OF_ORDER     = 0x01,
	
	/* Only sent over the wire, not present in getLatestPacket() return value */
	PKTFLAG_CONTINUED        = 0x02,
	
	/* Both sent over the wire and present in getLatestPacket() return value */
	PKTFLAG_FINISHED         = 0x04,
	PKTFLAG_RETRANSMISSION   = 0x08,
	PKTFLAG_STATE_CHANGE     = 0x10,
	PKTFLAG_STATE_CHANGE_ACK = 0x20,
};

struct  ConnectionCommon {
	ConnectionCommon(const NetAddress& remote_addr);
	size_t getHeaderSize() const { return 4; }
	void setTransform(PacketTransformation* transform);
	const NetAddress& remote_addr;
	uint16_t seq_num;
	PacketTransformation* transform;
};

class ConnectionIn {
public:
	ConnectionIn(const NetAddress& remote_addr);
	bool addPacket(Packet& p);
	const NetAddress& getAddress() const;
	bool hasNewPacket() const;
	PacketFlags getLatestPacket(Packet& p);
	 void setTransform(PacketTransformation* transform);
private:
	ConnectionCommon cc;
	bool new_packet, full_packet;
	std::bitset<NRG_CONN_PACKET_HISTORY> packet_history;
	PacketFlags latest_flags;
	Packet latest, buffer;
};

class ConnectionOut {
public:
	ConnectionOut(const NetAddress& remote_addr, const Socket& sock_out);
	void sendPacket(Packet& p, PacketFlags f = PKTFLAG_NONE);
	void sendDisconnect(Packet& extra_data);
	 void setTransform(PacketTransformation* transform);
	bool resendLastPacket(void);
private:
	ConnectionCommon cc;
	const Socket& sock;
	Packet buffer, buffer2;
};

struct Connection {
	Connection(const NetAddress& remote_addr, const Socket& sock_out)
		: in(remote_addr), out(remote_addr, sock_out){}
	ConnectionIn in;
	ConnectionOut out;
};

}

#endif
