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

ssize_t nrg::Socket::sendPacket(const Packet& p){
	return ::send(fd, p.data, p.data_size, 0);
}

ssize_t nrg::Socket::sendPacket(const Packet& p, const NetAddress& addr){
	socklen_t len = 0;
	const struct sockaddr* sa = addr.toSockAddr(len);
	return ::sendto(fd, p.data, p.data_size, 0, sa, len);
}

ssize_t nrg::Socket::recvPacket(Packet& p){
	return ::recv(fd, p.data, p.data_size, 0);
}

ssize_t nrg::Socket::recvPacket(Packet& p, NetAddress& addr){
	struct sockaddr_storage sas;
	socklen_t len = 0;
	ssize_t r = ::recvfrom(fd, p.data, p.data_size, 0, (struct sockaddr*)&sas, &len);
	addr.set(sas, len);
	return r;
}

nrg::UDPSocket::UDPSocket(int family) : nrg::Socket(family, SOCK_DGRAM){

}



