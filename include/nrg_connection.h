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
	struct ReassemblyInfo {
		ReassemblyInfo() : data(), age(-1), continued(false){}
		Packet data;
		int age;
		bool continued;
	};
	std::array<ReassemblyInfo, NRG_CONN_PACKET_HISTORY> reassembly_buf;
};

class ConnectionOut {
public:
	ConnectionOut(const NetAddress& remote_addr, const Socket& sock_out);
	Status sendPacket(Packet& p, PacketFlags f = PKTFLAG_NONE);
	Status sendDisconnect(Packet& extra_data);
	Status resendLastPacket(void);
	Status getLastStatus() const;
	void setTransform(PacketTransformation* transform);
private:
	Status sendPacketWithHeader(Packet& p, PacketHeader h);
	ConnectionCommon cc;
	const Socket& sock;
	Packet buffer, buffer2, last;
	PacketHeader last_header;
	Status last_status;
};

struct Connection {
	Connection(const NetAddress& remote_addr, const Socket& sock_out)
		: in(remote_addr), out(remote_addr, sock_out){}
	ConnectionIn in;
	ConnectionOut out;
};

}

#endif
