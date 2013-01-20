#include "nrg_netaddress.h"
#include <cstddef>
#include <cstring>
#include <cstdio>

static const off_t addr_off = offsetof(struct sockaddr_in, sin_addr);
static const off_t addr6_off = offsetof(struct sockaddr_in6, sin6_addr);

nrg::NetAddress::NetAddress() : text(), addr_len(0) {
	memset(&addr, 0, sizeof(addr));
};

nrg::NetAddress::NetAddress(const char* name, const char* port) : text(), addr_len(0) {
	struct addrinfo* result = NULL;
	int err = 0;

	if((err = getaddrinfo(name, port, NULL, &result)) == 0){
		addr_len = result->ai_addrlen;
		memcpy(&addr, result->ai_addr, addr_len);

		freeaddrinfo(result);
	}
}

nrg::NetAddress::NetAddress(const struct sockaddr_in& in) : addr_len(sizeof(in)) {
	memcpy(&addr, &in, addr_len);
}

nrg::NetAddress::NetAddress(const struct sockaddr_in6& in6) : addr_len(sizeof(in6)) {
	memcpy(&addr, &in6, addr_len);
}

nrg::NetAddress::NetAddress(const struct sockaddr_storage& s, const socklen_t len){
	set(s, len);
}

nrg::status_t nrg::NetAddress::set(const struct sockaddr_storage& s, const socklen_t len) {
	if(s.ss_family != AF_INET && s.ss_family != AF_INET6){
		return status::ERROR;
	}
	addr_len = len;
	memcpy(&addr, &s, len);
	return status::OK;
}

bool nrg::NetAddress::isValid() const {
	return addr_len != 0;
}

const char* nrg::NetAddress::name() {
	if(!text[0] && isValid()){
		off_t o;
		if(addr.ss_family == AF_INET){
			o = addr_off;
		} else if(addr.ss_family == AF_INET6){
			o = addr6_off;
		} else {
			return text;
		}
		inet_ntop(addr.ss_family, (char*)&addr + o, text, INET6_ADDRSTRLEN);
	}
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

};
