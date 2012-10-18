#ifndef NRG_NETADDRESS_H
#define NRG_NETADDRESS_H
#include "nrg_core.h"

namespace nrg {

class NRG_LIB NetAddress {
public:
	NetAddress(const char* name, const char* port);
	explicit NetAddress(const struct sockaddr_in& in);
	explicit NetAddress(const struct sockaddr_in6& in6);
	friend bool operator==(const NetAddress& a, const NetAddress& b);
	friend bool operator!=(const NetAddress& a, const NetAddress& b);
	const char* name() const;
	int family() const;
	const struct sockaddr* toSockAddr(socklen_t& out_size) const;
protected:
	char text[INET6_ADDRSTRLEN];
	struct sockaddr_storage addr;
	socklen_t addr_len;
};

};

#endif
