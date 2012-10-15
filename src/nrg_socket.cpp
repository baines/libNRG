#include "nrg_socket.h"

using nrg::status_t;

nrg::Socket::Socket(int domain, int type) 
: fd(0), domain(domain), type(type), error(false) {
	fd = socket(domain, type, 0);
	if(fd == -1) error = true;
}

status_t nrg::Socket::bind(const NetAddress& addr, uint16_t port){
	socklen_t len = 0;
	struct sockaddr* sa = addr.toSockAddr(port, len);
	if(::bind(fd, sa, len) == 0){
		return status::OK;
	} else {
		return status::ERROR;
	}
}

status_t nrg::Socket::connect(const NetAddress& addr, uint16_t port){
	socklen_t len = 0;
	struct sockaddr* sa = addr.toSockAddr(port, len);
	if(::connect(fd, sa, len) == 0){
		return status::OK;
	} else {
		return status::ERROR;
	}
}

status_t nrg::Socket::sendPacket(const PacketOut& p){
	return status::NYI;
}

status_t nrg::Socket::recvPacket(PacketIn& p){
	return status::NYI;
}

nrg::UDPSocket::UDPSocket() : nrg::Socket(AF_INET, SOCK_DGRAM){

}



