/*
  LibNRG - Networking for Real-time Games

  Copyright (C) 2012-2014 Alex Baines <alex@abaines.me.uk>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
/** @file
 *  Contains functionality that adds a connection abstraction over UDP
 */
#ifndef NRG_CONNECTION_H
#define NRG_CONNECTION_H
#include "nrg_core.h"
#include "nrg_netaddress.h"
#include "nrg_packet.h"
#include "nrg_packet_header.h"
#include "nrg_socket.h"
#include <bitset>
#include <array>

namespace nrg {

/** Attributes of Packets that are passed through ConnectionIn and ConnectionOut */
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

/** Common connection functionality that is used by both ConnectionIn and ConnectionOut */
struct ConnectionCommon {
	/** Default Constructor */
	ConnectionCommon();

	/** Add a PacketTransformation that will be applied to packets, or nullptr to disable */
	void setTransform(PacketTransformation* transform);

	uint16_t seq_num;
	PacketTransformation* transform;
};

/** Incoming connection class */
class ConnectionIn {
public:
	/** Default Constructor */
	ConnectionIn();

	/** Add a received packet with connection header information to be processed */
	bool addPacket(Packet& p);

	/** Returns true if there is a packet ready to be taken */
	bool hasNewPacket() const;

	/** Placed the latest packet into \p p, and returns its associated PacketFlags */
	PacketFlags getLatestPacket(Packet& p);

	/** Set a PacketTransformation to be removed from packets added to the connection */
	void setTransform(PacketTransformation* transform);
private:
	ConnectionCommon cc;
	bool new_packet, full_packet;
	std::bitset<NRG_CONN_PACKET_HISTORY> packet_history;
	PacketFlags latest_flags;
	Packet latest, buffer;
	struct ReassemblyInfo {
		ReassemblyInfo() : data(), age(-1), continued(false){}
		Packet data;
		int age;
		bool continued;
	};
	std::array<ReassemblyInfo, NRG_CONN_PACKET_HISTORY> reassembly_buf;
};

/** Outgoing connection class */
class ConnectionOut {
public:
	/** Create a new ConnectionOut that will send packets to \p remote_addr using the Socket \p sock_out */
	ConnectionOut(const NetAddress& remote_addr, const Socket& sock_out);

	/** Send Packet \p p, prepending header information including the given flags \p f, and applying any transformation */
	Status sendPacket(Packet& p, PacketFlags f = PKTFLAG_NONE);

	/** Send a Packet informing the remote host that the connection is over, \p extra_data can be a message explaining why */
	Status sendDisconnect(Packet& extra_data);

	/** Resends the last packet that was sent via this ConnectionOut instance */
	Status resendLastPacket(void);

	/** Gets the Status that the last sending operation returned */
	Status getLastStatus() const;

	/** Set a PacketTransformation to be removed from packets added to the connection */
	void setTransform(PacketTransformation* transform);
private:
	Status sendPacketWithHeader(Packet& p, PacketHeader h);
	ConnectionCommon cc;
	const NetAddress& remote_addr;
	const Socket& sock;
	Packet buffer, buffer2, last;
	PacketHeader last_header;
	Status last_status;
};

/** Combines both ConnectionIn and ConnectionOut into a single class */
struct Connection {
	/** Standard Constructor */
	Connection(const NetAddress& remote_addr, const Socket& sock_out)
		: in(), out(remote_addr, sock_out){}
	ConnectionIn in;
	ConnectionOut out;
};

}

#endif
