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
 * Classes to wrap POSIX sockets
 */
#ifndef NRG_SOCKET_H
#define NRG_SOCKET_H
#include "nrg_core.h"
#include "nrg_netaddress.h"
#include "nrg_packet.h"
#include <memory>

namespace nrg {

/** Base socket class */
class Socket {
public:
	/** Constructs a socket with the given type and family */
	Socket(int type, int family = PF_UNSPEC);
	/** Constructs a socket with the given type and a family derived from the given address */
	Socket(int type, const NetAddress& addr);

	/** Sets the socket's family using the family of the given address */
	void setFamilyFromAddress(const NetAddress& addr);

	/** Bind the socket to the given local address */
	Status bind(const NetAddress& addr);

	/** Connects the socket to the given remote address */
	Status connect(const NetAddress& addr);

	/** Disconnects the socket */
	void disconnect();

	/** Returns true if the socket is connected */
	bool isConnected() const;

	/** Sends a Packet to the address this socket is connected to */
	Status sendPacket(const Packet& p) const;

	/** Sends a Packet to the given address */
	Status sendPacket(const Packet& p, const NetAddress& addr) const;

	/** Receive a Packet from this socket's connected address */
	Status recvPacket(Packet& p) const;

	/** Receive a Packet from anyone, and put their address into addr */
	Status recvPacket(Packet& p, NetAddress& addr);

	/** Template wrapper around setsockopt */
	template<typename T>
	bool setOption(int level, int name, const T& opt){
		 return setsockopt(fd, level, name, &opt, sizeof(T)) == 0;
	}

	/** Uses select() to determine if data is available for reading on this socket */
	bool dataPending(int usToBlock = 0) const;

	/** Sets this socket as non-blocking */
	void setNonBlocking(bool nonblock);

	/** Enables or disables the use of getLastTimestamp() */
	void enableTimestamps(bool enable);

	/** Enables or disables checking for ICMP related errors on this socket, if available */
	void handleUnconnectedICMPErrors(bool enable);

	/** Returns the address this socket is bound to */
	const std::unique_ptr<NetAddress>& getBoundAddress() const;

	/** Returns the address this socket is connected to */
	const std::unique_ptr<NetAddress>& getConnectedAddress() const {
		return connected_addr;
	}

	/** Gets a timestamp of when the last packet was received, if available and enabled */
	uint64_t getLastTimestamp() const {
		return last_timestamp;
	}

	/** Default destructor */
	virtual ~Socket();
private:
	Status checkErrorQueue(NetAddress& culprit);

	mutable std::unique_ptr<NetAddress> bound_addr, connected_addr;
	int fd, family, type;
	bool do_timestamp, use_errqueue;
	uint64_t last_timestamp;
};

/** Socket derivative specifically for the User-Datagram Protocol */
struct UDPSocket : public Socket {
	UDPSocket(int family = PF_UNSPEC);
	UDPSocket(const NetAddress& a);
};

}

#endif
