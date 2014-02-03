#ifndef NRG_SOCKET_H
#define NRG_SOCKET_H
#include "nrg_core.h"
#include "nrg_netaddress.h"
#include "nrg_packet.h"
#include <memory>

namespace nrg {

class NRG_LIB Socket {
public:
	Socket(int type, int family = PF_UNSPEC);
	Socket(int type, const NetAddress& addr);
	bool bind(const NetAddress& addr);
	bool connect(const NetAddress& addr);
	ssize_t sendPacket(const Packet& p) const;
	ssize_t sendPacket(const Packet& p, const NetAddress& addr) const;
	ssize_t recvPacket(Packet& p) const;
	ssize_t recvPacket(Packet& p, NetAddress& addr);
	template<typename T>
	bool setOption(int level, int name, const T& opt){
		 return setsockopt(fd, level, name, &opt, sizeof(T)) == 0;
	}
	bool dataPending(int usToBlock = 0) const;
	void setNonBlocking(bool nonblock);
	void enableTimestamps(bool enable);
	
	const NetAddress* getBoundAddress();
	const NetAddress* getBoundAddress() const;
	const NetAddress* getConnectedAddress() const {
		return connected_addr;
	}
	uint64_t getLastTimestamp() const {
		return last_timestamp;
	}
	~Socket();
protected:
	NetAddress *bound_addr, *connected_addr;
	int fd, family, type;
	bool do_timestamp;
	uint64_t last_timestamp;
};

struct NRG_LIB UDPSocket : public Socket {
	UDPSocket(int family = PF_UNSPEC);
	UDPSocket(const NetAddress& a);
};

}

#endif
