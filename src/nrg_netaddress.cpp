#include "nrg_netaddress.h"
#include <cstddef>
#include <cstring>
#include <cstdio>

nrg::NetAddress::NetAddress(const char* name) : addr_len(0) {
	struct addrinfo* result = NULL;
	if(getaddrinfo(name, NULL, NULL, &result) == 0){
		addr_len = result->ai_addrlen;
		memcpy(&addr, result->ai_addr, addr_len);
		off_t o = 0;
		if(addr.ss_family == AF_INET){
			o = offsetof(struct sockaddr_in, sin_addr);
		} else if(addr.ss_family == AF_INET6){
			o = offsetof(struct sockaddr_in6, sin6_addr);
		}
		inet_ntop(addr.ss_family, (char*)&addr + o, text, INET6_ADDRSTRLEN);
		freeaddrinfo(result);
	}
}

nrg::NetAddress::NetAddress(const struct in_addr& in) : addr_len(0) {
	struct sockaddr_in sai = { AF_INET, 0, in };
	addr_len = sizeof(sai);	
	memcpy(&addr, &sai, addr_len);
	inet_ntop(AF_INET, (char*)&addr + offsetof(struct sockaddr_in, sin_addr)
	                                                  , text, INET6_ADDRSTRLEN);
}

const char* nrg::NetAddress::name() const {
	return text;
}

int nrg::NetAddress::family() const {
	return addr.ss_family;
}

struct sockaddr* nrg::NetAddress::toSockAddr(uint16_t port, socklen_t& out_size) const{
	struct sockaddr* in;
	if(addr.ss_family == AF_INET){
		struct sockaddr_in* i = new struct sockaddr_in();
		memcpy(i, &addr, addr_len);
		i->sin_port = htons(port);
		in = reinterpret_cast<struct sockaddr*>(i);
	} else if(addr.ss_family == AF_INET6){
		struct sockaddr_in6* i = new struct sockaddr_in6();
		memcpy(i, &addr, addr_len);
		i->sin6_port = htons(port);
		in = reinterpret_cast<struct sockaddr*>(i);
	}

	out_size = addr_len;
	return in;
}
