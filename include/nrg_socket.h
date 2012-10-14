#ifndef NRG_SOCKET_H
#define NRG_SOCKET_H
#include "nrg_core.h"
#include "nrg_packet.h"

namespace nrg {

class Socket {
public:
	Socket();
	explicit Socket(uint16_t bind_port);
	Socket(const NetAddress& host, uint16_t port);
	status_t bind(uint16_t port);
	status_t connect(const NetAddress& host, uint16_t port);
	template<typename T>
	status_t setOption(int level, int name, const T& opt){
		setsockopt(fd, level, name, &opt, sizeof(T));
	}
	int recvPacket(PacketIn& p);
	int sendPacket(const PacketOut& p);
protected:
	int fd;
};

class UDPSocket {
public:
	UDPSocket();
};

};

#endif
