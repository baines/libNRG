#include "nrg_netaddress.h"
#include "nrg_endian.h"
#include <cstddef>
#include <cstring>
#include <cstdio>

using namespace nrg;

static const off_t addr_off = offsetof(struct sockaddr_in, sin_addr);
static const off_t addr6_off = offsetof(struct sockaddr_in6, sin6_addr);

NetAddress::NetAddress() 
: text()
, addr()
, addr_len(0) {

}

NetAddress::NetAddress(const char* name, const char* port) : text(), addr(), addr_len(0) {
	resolve(name, port);
}

NetAddress::NetAddress(const struct sockaddr_in& in) : addr_len(sizeof(in)) {
	memcpy(&addr, &in, addr_len);
	inet_ntop(AF_INET, (char*)&addr + addr_off, text, INET6_ADDRSTRLEN);
}

NetAddress::NetAddress(const struct sockaddr_in6& in6) : addr_len(sizeof(in6)) {
	memcpy(&addr, &in6, addr_len);
	inet_ntop(AF_INET6, (char*)&addr + addr6_off, text, INET6_ADDRSTRLEN);
}

NetAddress& NetAddress::operator=(const struct sockaddr_storage& s){
	set(s);
	return *this;
}

NetAddress::NetAddress(const struct sockaddr_storage& s){
	set(s);
}

bool NetAddress::set(const struct sockaddr_storage& s) {
	off_t o = 0;
	if(s.ss_family == AF_INET){
		o = addr_off;
		addr_len = sizeof(struct sockaddr_in);
	} else if(s.ss_family == AF_INET6){
		o = addr6_off;
		addr_len = sizeof(struct sockaddr_in6);
	} else {
		return false;
	}
	addr = s;
	inet_ntop(s.ss_family, (char*)&addr + o, text, INET6_ADDRSTRLEN);
	return true;
}

bool NetAddress::set(const struct sockaddr& s) {
	off_t o = 0;
	if(s.sa_family == AF_INET){
		o = addr_off;
		addr_len = sizeof(struct sockaddr_in);
	} else if(s.sa_family == AF_INET6){
		o = addr6_off;
		addr_len = sizeof(struct sockaddr_in6);
	} else {
		puts("That's no mo.. sockaddr!");
		return false;
	}
	memcpy(&addr, &s, addr_len);
	inet_ntop(s.sa_family, (char*)&addr + o, text, INET6_ADDRSTRLEN);
	return true;
}

bool NetAddress::resolve(const char* name, const char* port){
	struct addrinfo *result = NULL, hints = {};
	hints.ai_family = AF_UNSPEC;
	hints.ai_flags = (AI_ALL | AI_V4MAPPED);
	
	int err = 0;

	if((err = getaddrinfo(name, port, &hints, &result)) == 0){
		addr_len = result->ai_addrlen;
		memcpy(&addr, result->ai_addr, addr_len);

		off_t o = result->ai_family == AF_INET ? addr_off : addr6_off;
		inet_ntop(result->ai_family, (char*)&addr + o, text, INET6_ADDRSTRLEN);

		freeaddrinfo(result);
	}

	return isValid();
}

bool NetAddress::isValid() const {
	return addr_len != 0;
}

const char* NetAddress::name() const {
	return text;
}

int NetAddress::family() const {
	return addr.ss_family;
}

uint16_t NetAddress::port() const {
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
