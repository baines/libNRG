#include "nrg_socket.h"
#include "nrg_config.h"

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

ssize_t nrg::Socket::sendPacket(const Packet& p) const {
	return ::send(fd, p.data, p.data_size, 0);
}

ssize_t nrg::Socket::sendPacket(const Packet& p, const NetAddress& addr) const {
	socklen_t len = 0;
	const struct sockaddr* sa = addr.toSockAddr(len);
	return ::sendto(fd, p.data, p.data_size, 0, sa, len);
}

ssize_t nrg::Socket::recvPacket(Packet& p) const {
	uint8_t buf[NRG_MAX_PACKET_SIZE];
	ssize_t result = ::recv(fd, buf, NRG_MAX_PACKET_SIZE, 0);
	if(result > 0){
		p.writeArray(buf, result);
	}
	return result;
}

ssize_t nrg::Socket::recvPacket(Packet& p, NetAddress& addr) const {
	struct sockaddr_storage sas;
	socklen_t len = 0;
	uint8_t buf[NRG_MAX_PACKET_SIZE];
	ssize_t result = ::recvfrom(fd, buf, NRG_MAX_PACKET_SIZE, 0, (struct sockaddr*)&sas, &len);
	if(result > 0){
		p.writeArray(buf, result);
	}
	addr.set(sas, len);
	return result;
}

nrg::UDPSocket::UDPSocket(int family) : nrg::Socket(family, SOCK_DGRAM){

}



