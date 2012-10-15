#ifndef NRG_NETADDRESS_H
#define NRG_NETADDRESS_H
#include "nrg_core.h"

class NetAddress {
public:
	NetAddress(const char* name);
	NetAddress(const struct in_addr& in);
	NetAddress(const struct in6_addr& in6);
	friend bool operator==(const NetAddress& a, const NetAddress& b);
	friend bool operator!=(const NetAddress& a, const NetAddress& b);
	const char* name() const;
	struct sockaddr* toSockAddr(uint16_t port, socklen_t& outSize) const;
	const int domain; //AF_INET or AF_INET6
protected:
	char text[INET6_ADDRSTRLEN];
	union addr {
		struct in_addr in;
		struct in6_addr in6;
	};
};

#endif
