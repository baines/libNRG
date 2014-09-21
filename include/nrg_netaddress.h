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
 *  Contains the NetAddress class for wrapping POSIX sockaddr structures and resolving hostnames to IP addresses
 */
#ifndef NRG_NETADDRESS_H
#define NRG_NETADDRESS_H
#include "nrg_core.h"

namespace nrg {

/** Class to wrap the various POSIX sockaddrs and resolve hostnames */
class NetAddress {
public:
	/** Default constructor */
	NetAddress();
	
	/** Construct and resolve an address from IP/Hostname and Port/Service */
	NetAddress(const char* name, const char* port);
	
	/** Implicitly construct from a IPv4 sockaddr_in */
	NetAddress(const struct sockaddr_in& in);
	
	/** Implicitly construct from a IPv6 sockaddr_in6 */
	NetAddress(const struct sockaddr_in6& in6);
	
	/** Implicitly construct from a sockaddr_storage */
	NetAddress(const struct sockaddr_storage& s);
	
	/** Implicitly construct from a generic sockaddr */
	NetAddress(const struct sockaddr& s);

	/** Resolve the given IP/Hostname and Port/Service into an address, returning true on success */
	bool resolve(const char* name, const char* port);

	/** Queries whether or not the contained address is valid */
	bool isValid() const;	
	
	/** Returns the contained IP address as a string, if no address is contained it returns "" - do not free */
	const char* getIP() const;
	
	/** Returns the family of the contained address, i.e. AF_INET, AF_INET6 or AF_UNSPEC */
	int getFamily() const;
	
	/** Returns the contained address' port as a short in host byte order */
	uint16_t getPort() const;
	
	/** Returns a sockaddr* representation of the NetAddress to be used with POSIX socket functions - do not free */
	const struct sockaddr* toSockAddr(socklen_t& out_size) const;
	
	/** Equality operator */
	friend bool operator==(const NetAddress& a, const NetAddress& b);
	/** Unequality operator */
	friend bool operator!=(const NetAddress& a, const NetAddress& b);
	/** Less-than operator for total-ordered containers like std::map */
	friend bool operator<(const NetAddress& a, const NetAddress& b);
private:
	mutable char text[INET6_ADDRSTRLEN];
	struct sockaddr_storage addr;
	socklen_t addr_len;
};

}

#endif
