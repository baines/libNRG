#ifndef NRG_SOCKET_H
#define NRG_SOCKET_H
#include "nrg_core.h"
#include "nrg_netaddress.h"
#include "nrg_packet.h"
#include <memory>

namespace nrg {

class NRG_LIB Socket {
public:
	Socket(int family, int type);
	status_t bind(const NetAddress& addr);
	status_t connect(const NetAddress& addr);
	ssize_t sendPacket(const Packet& p) const;
	ssize_t sendPacket(const Packet& p, const NetAddress& addr) const;
	ssize_t recvPacket(Packet& p) const;
	ssize_t recvPacket(Packet& p, NetAddress& addr) const;
	template<typename T>
	status_t setOption(int level, int name, const T& opt){
		if(setsockopt(fd, level, name, &opt, sizeof(T)) == 0){
			return status::OK;
		} else {
			return status::ERROR;
		}
	}
	bool dataPending(int usToBlock = 0) const;
	void setNonBlocking(bool nonblock);
	const NetAddress* getBoundAddress() {
		if(bound_addr.get() == NULL){
			struct sockaddr_storage sas;
			socklen_t len = sizeof(sas);
			if(getsockname(fd, (struct sockaddr*)&sas, &len) == 0){
				bound_addr = std::auto_ptr<NetAddress>(new NetAddress(sas, len));
			}
		}
		return bound_addr.get();
	}
	const NetAddress* getConnectedAddress() const {
		return connected_addr.get();
	}
protected:
	std::auto_ptr<NetAddress> bound_addr, connected_addr;
	int fd, family, type;
	bool error;
};

struct NRG_LIB UDPSocket : public Socket {
	UDPSocket(int family = AF_INET);
};

};

#endif
