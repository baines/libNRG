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
#include "nrg_netaddress.h"
#include "nrg_endian.h"
#include "nrg_os.h"
#include <cstddef>
#include <cstring>
#include <cstdio>

using namespace nrg;

NetAddress::NetAddress() 
: text()
, addr()
, addr_len(0) {

}

NetAddress::NetAddress(const char* name, const char* port)
: text()
, addr()
, addr_len(0) {
	resolve(name, port);
}

NetAddress::NetAddress(const struct sockaddr_in& in)
: addr_len(sizeof(in)) {
	memcpy(&addr, &in, addr_len);
}

NetAddress::NetAddress(const struct sockaddr_in6& in6)
: addr_len(sizeof(in6)) {
	memcpy(&addr, &in6, addr_len);
}

NetAddress::NetAddress(const struct sockaddr_storage& s)
: text()
, addr(s)
, addr_len(0){
	if(s.ss_family == AF_INET){
		addr_len = sizeof(struct sockaddr_in);
	} else if(s.ss_family == AF_INET6){
		addr_len = sizeof(struct sockaddr_in6);
	}
}

NetAddress::NetAddress(const struct sockaddr& s)
: text()
, addr()
, addr_len(0){
	if(s.sa_family == AF_INET){
		addr_len = sizeof(struct sockaddr_in);
	} else if(s.sa_family == AF_INET6){
		addr_len = sizeof(struct sockaddr_in6);
	}
	memcpy(&addr, &s, addr_len);
}

bool NetAddress::resolve(const char* name, const char* port){
	struct addrinfo *result = NULL, *hintp = NULL;

#ifndef _WIN32
	struct addrinfo hints = {};
	hints.ai_family = AF_UNSPEC;
	hints.ai_flags = (AI_ALL | AI_V4MAPPED);
	hintp = &hints;
#endif

	int err = 0;

	if((err = getaddrinfo(name, port, hintp, &result)) == 0){
		addr_len = result->ai_addrlen;
		memcpy(&addr, result->ai_addr, addr_len);

		freeaddrinfo(result);
	}

	return isValid();
}

bool NetAddress::isValid() const {
	return addr_len != 0;
}

const char* NetAddress::getIP() const {
	if(!*text){
		getnameinfo(
			reinterpret_cast<const sockaddr*>(&addr),
			addr_len,
			text,
			INET6_ADDRSTRLEN,
			NULL,
			0,
			NI_NUMERICHOST
		);
	}
	return text;
}

int NetAddress::getFamily() const {
	return addr.ss_family;
}

uint16_t NetAddress::getPort() const {
	if(addr.ss_family == AF_INET){
		return ntoh(reinterpret_cast<const struct sockaddr_in*>(&addr)->sin_port);
	} else if(addr.ss_family == AF_INET6){
		return ntoh(reinterpret_cast<const struct sockaddr_in6*>(&addr)->sin6_port);
	} else {
		return 0;
	}
}

const struct sockaddr* NetAddress::toSockAddr(socklen_t& out_size) const {
	out_size = addr_len;
	return reinterpret_cast<const struct sockaddr*>(&addr);
}

namespace nrg {

bool operator==(const NetAddress& a, const NetAddress& b){
	if(a.addr_len == b.addr_len){
		return (memcmp(&a.addr, &b.addr, a.addr_len) == 0);
	} else {
		return false;
	}
}

bool operator!=(const NetAddress& a, const NetAddress& b){
	return !(a == b);
}

bool operator<(const NetAddress& a, const NetAddress& b){
	if(a.addr_len != b.addr_len){
		return a.addr_len < b.addr_len;
	} else {
		return memcmp(&a.addr, &b.addr, a.addr_len) < 0;
	}
}

}
