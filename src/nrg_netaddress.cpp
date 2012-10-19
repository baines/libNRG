#include "nrg_netaddress.h"
#include <cstddef>
#include <cstring>
#include <cstdio>

static const off_t addr_off = offsetof(struct sockaddr_in, sin_addr);
static const off_t addr6_off = offsetof(struct sockaddr_in6, sin6_addr);

nrg::NetAddress::NetAddress(const char* name, const char* port) : addr_len(0) {
	struct addrinfo* result = NULL;

	if(getaddrinfo(name, port, NULL, &result) == 0){
		addr_len = result->ai_addrlen;
		memcpy(&addr, result->ai_addr, addr_len);
	
		off_t o = result->ai_family == AF_INET ? addr_off : addr6_off;
		inet_ntop(result->ai_family, (char*)&addr + o, text, INET6_ADDRSTRLEN);

		freeaddrinfo(result);
	}
}

nrg::NetAddress::NetAddress(const struct sockaddr_in& in) : addr_len(sizeof(in)) {
	memcpy(&addr, &in, addr_len);
	inet_ntop(AF_INET, (char*)&addr + addr_off, text, INET6_ADDRSTRLEN);
}

nrg::NetAddress::NetAddress(const struct sockaddr_in6& in6) : addr_len(sizeof(in6)) {
	memcpy(&addr, &in6, addr_len);
	inet_ntop(AF_INET6, (char*)&addr + addr6_off, text, INET6_ADDRSTRLEN);
}

const char* nrg::NetAddress::name() const {
	return text;
}

int nrg::NetAddress::family() const {
	return addr.ss_family;
}

uint16_t nrg::NetAddress::port() const {
	if(addr.ss_family == AF_INET){
		return ntohs(reinterpret_cast<const struct sockaddr_in*>(&addr)->sin_port);
	} else if(addr.ss_family == AF_INET6){
		return ntohs(reinterpret_cast<const struct sockaddr_in6*>(&addr)->sin6_port);
	} else {
		return 0;
	}
}

const struct sockaddr* nrg::NetAddress::toSockAddr(socklen_t& out_size) const {
	out_size = addr_len;
	return reinterpret_cast<const struct sockaddr*>(&addr);
}
