#include "nrg_socket.h"
#include "nrg_config.h"
#include "nrg_os.h"
#include <unistd.h>
#include <sys/time.h>

nrg::Socket::Socket(int type, int family) : bound_addr(NULL), connected_addr(NULL), 
fd(0), family(family), type(type), do_timestamp(false), last_timestamp(0) {
	if(family != PF_UNSPEC) fd = socket(family, type, 0);
}

nrg::Socket::Socket(int type, const nrg::NetAddress& a) : bound_addr(NULL), connected_addr(NULL),
fd(0), family(a.family()), type(type), do_timestamp(false), last_timestamp(0) {
	fd = socket(family, type, 0);
}

static inline void addrAssign(nrg::NetAddress*& oldaddr, nrg::NetAddress* const& newaddr){
	if(oldaddr != NULL)	delete oldaddr;
	oldaddr = newaddr;
}

bool nrg::Socket::bind(const NetAddress& addr){
	if(!fd){
		family = addr.family();
		fd = socket(family, type, 0);
	} else if(addr.family() != family) return status::ERROR;
	socklen_t len = 0;
	
	const struct sockaddr* sa = addr.toSockAddr(len);
	if(::bind(fd, sa, len) == 0){
		addrAssign(bound_addr, new NetAddress(addr));
		return true;
	} else {
		return false;
	}
}

bool nrg::Socket::connect(const NetAddress& addr){
	if(!fd){
		family = addr.family();
		fd = socket(family, type, 0);
	} else if(addr.family() != family) return status::ERROR;
	socklen_t len = 0;

	const struct sockaddr* sa = addr.toSockAddr(len);
	if(::connect(fd, sa, len) == 0){
		addrAssign(connected_addr, new NetAddress(addr));
		return true;
	} else {
		return false;
	}
}

ssize_t nrg::Socket::sendPacket(const Packet& p) const {
	return ::send(fd, p.data, p.used_size, 0);
}

ssize_t nrg::Socket::sendPacket(const Packet& p, const NetAddress& addr) const {
	socklen_t len = 0;
	const struct sockaddr* sa = addr.toSockAddr(len);
	return ::sendto(fd, p.data, p.used_size, 0, sa, len);
}

ssize_t nrg::Socket::recvPacket(Packet& p) const {
	uint8_t buf[NRG_MAX_PACKET_SIZE];
	ssize_t result = ::recv(fd, buf, NRG_MAX_PACKET_SIZE, 0);
	if(result > 0){
		p.writeArray(buf, result);
	}
	return result;
}

ssize_t nrg::Socket::recvPacket(Packet& p, NetAddress& addr) {
	struct sockaddr_storage sas = {};
	uint8_t buf[NRG_MAX_PACKET_SIZE];
	socklen_t len = sizeof(sas);

#ifdef NRG_USE_SO_TIMESTAMP
	struct msghdr msg = {};
    struct iovec iov = {};

	char cbuf[CMSG_SPACE(sizeof(struct timeval))];
	struct cmsghdr* cmsg = (struct cmsghdr*)cbuf;

	iov.iov_base = buf;
	iov.iov_len = sizeof(buf);

	msg.msg_name = &sas;
	msg.msg_namelen = len;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control = cmsg;
	msg.msg_controllen = sizeof(cbuf);

	ssize_t result = ::recvmsg(fd, &msg, 0);
#else
	ssize_t result = ::recvfrom(fd, buf, sizeof(buf), 0, (sockaddr*)&sas, &len);
#endif
	if(result > 0){
		p.writeArray(buf, result);
		if(do_timestamp){ 
			last_timestamp = os::microseconds();
#ifdef NRG_USE_SO_TIMESTAMP
			for(cmsg = CMSG_FIRSTHDR(&msg); cmsg; cmsg = CMSG_NXTHDR(&msg, cmsg)){
				if(cmsg->cmsg_level == SOL_SOCKET && cmsg->cmsg_type == SO_TIMESTAMP){
					struct timeval *tv1 = (struct timeval*)CMSG_DATA(cmsg), tv2, tv3;
					gettimeofday(&tv2, 0);
					timersub(&tv2, tv1, &tv3);
					last_timestamp -= tv3.tv_usec;
				}
			}
#endif
		}
		addr.set(sas);
	}
	return result;
}

bool nrg::Socket::dataPending(int usToBlock) const {
	fd_set s;
	FD_ZERO(&s);
	FD_SET(fd, &s);
	struct timeval tv = { 0, usToBlock };

	if(select(fd+1, &s, NULL, NULL, &tv) == 1){
		return true;
	} else {
		return false;
	}
}

const nrg::NetAddress* nrg::Socket::getBoundAddress() const {
	return bound_addr;
}

const nrg::NetAddress* nrg::Socket::getBoundAddress(){
	if(bound_addr == NULL){
		struct sockaddr_storage sas = {}, sas_zero = {};
		socklen_t len = sizeof(sas);
		if(getsockname(fd, (struct sockaddr*)&sas, &len) == 0){
			sas_zero.ss_family = sas.ss_family;
			if(memcmp(&sas, &sas_zero, sizeof(sas)) != 0){
				bound_addr = new NetAddress(sas);
			}
		}
	}
	return bound_addr;
}

nrg::Socket::~Socket(){
	if(bound_addr) delete bound_addr;
	if(fd >= 0) close(fd);
}

#if defined __WIN32
#include <Winsock2.h>
void nrg::Socket::setNonBlocking(bool enabled){
	u_long l = enabled ? 1 : 0;
	ioctlsocket(fd, FIONBIO, &i);
}
#else
#include <fcntl.h>
void nrg::Socket::setNonBlocking(bool enabled){
	int flags = fcntl(fd, F_GETFL, 0);
	flags = (enabled ? flags | O_NONBLOCK : flags & ~O_NONBLOCK);
	fcntl(fd, F_SETFL, flags);
}
#endif

void nrg::Socket::enableTimestamps(bool enable){
	do_timestamp = enable;
}

nrg::UDPSocket::UDPSocket(int family) : nrg::Socket(SOCK_DGRAM, family){

}

nrg::UDPSocket::UDPSocket(const NetAddress& a) : nrg::Socket(SOCK_DGRAM, a){

}



