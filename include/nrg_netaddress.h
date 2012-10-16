#ifndef NRG_NETADDRESS_H
#define NRG_NETADDRESS_H
#include "nrg_core.h"

namespace nrg {

class NetAddress {
public:
	NetAddress(const char* name);
	NetAddress(const struct in_addr& in);
	//NetAddress(const struct in6_addr& in6);
	friend bool operator==(const NetAddress& a, const NetAddress& b);
	friend bool operator!=(const NetAddress& a, const NetAddress& b);
	const char* name() const;
	int family() const;
	struct sockaddr* toSockAddr(uint16_t port, socklen_t& out_size) const;
protected:
	char text[INET6_ADDRSTRLEN];
	struct sockaddr_storage addr;
	size_t addr_len;
};

};

#endif
