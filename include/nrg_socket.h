#ifndef NRG_SOCKET_H
#define NRG_SOCKET_H
#include "nrg_core.h"
#include "nrg_netaddress.h"
#include "nrg_packet.h"

namespace nrg {

class Socket {
public:
	Socket(int domain, int type);
	status_t bind(const NetAddress& addr, uint16_t port);
	status_t connect(const NetAddress& addr, uint16_t port);
	status_t sendPacket(const PacketOut& p);
	status_t recvPacket(PacketIn& p);
	template<typename T>
	status_t setOption(int level, int name, const T& opt){
		if(setsockopt(fd, level, name, &opt, sizeof(T)) == 0){
			return status::OK;
		} else {
			return status::ERROR;
		}
	}
protected:
	int fd, domain, type;
	bool error;
};

class UDPSocket : public Socket {
	UDPSocket();
};

};

#endif
