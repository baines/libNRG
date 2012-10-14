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
	const int domain; //AF_INET or AF_INET6
protected:
	char addr[sizeof(struct in6_addr)];
};

#endif
