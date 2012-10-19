#include "nrg_socket.h"

using nrg::status_t;

nrg::Socket::Socket(int family, int type) : bound_addr(), 
connected_addr(), fd(0), family(family), type(type), error(false) {
	fd = socket(family, type, 0);
	if(fd == -1) error = true;
}


status_t nrg::Socket::bind(const NetAddress& addr){
	if(addr.family() != family) return status::ERROR;
	socklen_t len = 0;

	const struct sockaddr* sa = addr.toSockAddr(len);
	if(::bind(fd, sa, len) == 0){
		bound_addr = std::auto_ptr<NetAddress>(new NetAddress(addr));
		return status::OK;
	} else {
		return status::ERROR;
	}
}

status_t nrg::Socket::connect(const NetAddress& addr){
	if(addr.family() != family) return status::ERROR;
	socklen_t len = 0;

	const struct sockaddr* sa = addr.toSockAddr(len);
	if(::connect(fd, sa, len) == 0){
		connected_addr = std::auto_ptr<NetAddress>(new NetAddress(addr));
		return status::OK;
	} else {
		return status::ERROR;
	}
}

status_t nrg::Socket::sendPacket(const Packet& p){
	return status::NYI;
}

status_t nrg::Socket::recvPacket(Packet& p){
	return status::NYI;
}

nrg::UDPSocket::UDPSocket(int family) : nrg::Socket(family, SOCK_DGRAM){

}



