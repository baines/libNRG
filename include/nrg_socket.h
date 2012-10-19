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
	status_t sendPacket(const Packet& p);
	status_t recvPacket(Packet& p);
	template<typename T>
	status_t setOption(int level, int name, const T& opt){
		if(setsockopt(fd, level, name, &opt, sizeof(T)) == 0){
			return status::OK;
		} else {
			return status::ERROR;
		}
	}
	const NetAddress* getBoundAddress(){
		return bound_addr.get();
	}
	const NetAddress* getConnectedAddress(){
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
