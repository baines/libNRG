#ifndef NRG_NETADDRESS_H
#define NRG_NETADDRESS_H
#include "nrg_core.h"

namespace nrg {

class NetAddress {
public:
	NetAddress();
	NetAddress(const char* name, const char* port);
	explicit NetAddress(const struct sockaddr_in& in);
	explicit NetAddress(const struct sockaddr_in6& in6);
	NetAddress(const struct sockaddr_storage& s);
	NetAddress& operator=(const struct sockaddr_storage& s);
	bool set(const struct sockaddr_storage& s);
	bool resolve(const char* name, const char* port);

	bool isValid() const;
	const char* name() const;
	int family() const;
	uint16_t port() const;
	const struct sockaddr* toSockAddr(socklen_t& out_size) const;

	friend bool operator==(const NetAddress& a, const NetAddress& b);
	friend bool operator!=(const NetAddress& a, const NetAddress& b);
	friend bool operator<(const NetAddress& a, const NetAddress& b);
private:
	char text[INET6_ADDRSTRLEN];
	struct sockaddr_storage addr;
	socklen_t addr_len;
};

}

#endif
