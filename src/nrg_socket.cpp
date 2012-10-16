#include "nrg_socket.h"

using nrg::status_t;

nrg::Socket::Socket(int family, int type) 
: fd(0), family(family), type(type), error(false) {
	fd = socket(family, type, 0);
	if(fd == -1) error = true;
}

status_t nrg::Socket::bind(const NetAddress& addr, uint16_t port){
	if(addr.family() != family) return status::ERROR;
	socklen_t len = 0;
	status_t res;

	struct sockaddr* sa = addr.toSockAddr(port, len);
	if(::bind(fd, sa, len) == 0){
		res = status::OK;
	} else {
		res = status::ERROR;
	}

	delete sa;
	return res;
}

status_t nrg::Socket::connect(const NetAddress& addr, uint16_t port){
	if(addr.family() != family) return status::ERROR;
	socklen_t len = 0;
	status_t res;

	struct sockaddr* sa = addr.toSockAddr(port, len);
	if(::connect(fd, sa, len) == 0){
		res = status::OK;
	} else {
		res = status::ERROR;
	}

	delete sa;
	return res;
}

status_t nrg::Socket::sendPacket(const PacketOut& p){
	return status::NYI;
}

status_t nrg::Socket::recvPacket(PacketIn& p){
	return status::NYI;
}

nrg::UDPSocket::UDPSocket() : nrg::Socket(AF_INET, SOCK_DGRAM){

}



