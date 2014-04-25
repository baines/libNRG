#ifndef NRG_SOCKET_H
#define NRG_SOCKET_H
#include "nrg_core.h"
#include "nrg_netaddress.h"
#include "nrg_packet.h"
#include <memory>

namespace nrg {

class Socket {
public:
	Socket(int type, int family = PF_UNSPEC);
	Socket(int type, const NetAddress& addr);
	
	void setFamilyFromAddress(const NetAddress& addr);
	
	Status bind(const NetAddress& addr);
	Status connect(const NetAddress& addr);
	void disconnect();
	bool isConnected() const;
	
	Status sendPacket(const Packet& p) const;
	Status sendPacket(const Packet& p, const NetAddress& addr) const;
	Status recvPacket(Packet& p) const;
	Status recvPacket(Packet& p, NetAddress& addr);
	
	template<typename T>
	bool setOption(int level, int name, const T& opt){
		 return setsockopt(fd, level, name, &opt, sizeof(T)) == 0;
	}
	
	Status checkErrorQueue(NetAddress& culprit);
		
	bool dataPending(int usToBlock = 0) const;
	void setNonBlocking(bool nonblock);
	void enableTimestamps(bool enable);
	void handleUnconnectedICMPErrors(bool enable);
	
	const std::unique_ptr<NetAddress>& getBoundAddress();
	const std::unique_ptr<NetAddress>& getBoundAddress() const;
	const std::unique_ptr<NetAddress>& getConnectedAddress() const {
		return connected_addr;
	}
	uint64_t getLastTimestamp() const {
		return last_timestamp;
	}
	virtual ~Socket();
private:
	std::unique_ptr<NetAddress> bound_addr, connected_addr;
	int fd, family, type;
	bool do_timestamp, use_errqueue;
	uint64_t last_timestamp;
};

struct  UDPSocket : public Socket {
	UDPSocket(int family = PF_UNSPEC);
	UDPSocket(const NetAddress& a);
};

}

#endif
