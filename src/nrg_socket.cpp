#include "nrg_socket.h"
#include "nrg_config.h"
#include "nrg_os.h"
#include <unistd.h>
#include <sys/time.h>

using namespace nrg;
using std::unique_ptr;

Socket::Socket(int type, int family) 
: bound_addr()
, connected_addr()
, fd(0)
, family(family)
, type(type)
, do_timestamp(false)
, last_timestamp(0) {
	if(family != PF_UNSPEC) fd = socket(family, type, 0);
}

Socket::Socket(int type, const NetAddress& a)
: bound_addr()
, connected_addr()
, fd(0)
, family(a.family())
, type(type)
, do_timestamp(false)
, last_timestamp(0) {
	fd = socket(family, type, 0);
}

void Socket::setFamilyFromAddress(const NetAddress& na){
	family = na.family();
	fd = socket(family, type, 0);
}

bool Socket::bind(const NetAddress& addr){
	if(!fd){
		family = addr.family();
		fd = socket(family, type, 0);
	} else if(addr.family() != family) return false;
	socklen_t len = 0;
	
	const struct sockaddr* sa = addr.toSockAddr(len);
	if(::bind(fd, sa, len) == 0){
		bound_addr = unique_ptr<NetAddress>(new NetAddress(addr));
		return true;
	} else {
		return false;
	}
}

bool Socket::connect(const NetAddress& addr){
	if(!fd){
		family = addr.family();
		fd = socket(family, type, 0);
	} else if(addr.family() != family) return false;
	socklen_t len = 0;

	const struct sockaddr* sa = addr.toSockAddr(len);
	if(::connect(fd, sa, len) == 0){
		connected_addr = unique_ptr<NetAddress>(new NetAddress(addr));
		return true;
	} else {
		return false;
	}
}

ssize_t Socket::sendPacket(const Packet& p) const {
	return ::send(fd, p.getBasePointer(), p.size(), 0);
}

ssize_t Socket::sendPacket(const Packet& p, const NetAddress& addr) const {
	socklen_t len = 0;
	const struct sockaddr* sa = addr.toSockAddr(len);
	return ::sendto(fd, p.getBasePointer(), p.size(), 0, sa, len);
}

ssize_t Socket::recvPacket(Packet& p) const {
	uint8_t buf[NRG_MAX_PACKET_SIZE];
	ssize_t result = ::recv(fd, buf, NRG_MAX_PACKET_SIZE, 0);
	if(result > 0){
		p.writeArray(buf, result);
	}
	return result;
}

ssize_t Socket::recvPacket(Packet& p, NetAddress& addr) {
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
		p.seek(0, SEEK_SET);
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

bool Socket::dataPending(int usToBlock) const {
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

const std::unique_ptr<NetAddress>& Socket::getBoundAddress() const {
	return bound_addr;
}

const std::unique_ptr<NetAddress>& Socket::getBoundAddress(){
	if(bound_addr){
		struct sockaddr_storage sas = {}, sas_zero = {};
		socklen_t len = sizeof(sas);
		if(getsockname(fd, (struct sockaddr*)&sas, &len) == 0){
			sas_zero.ss_family = sas.ss_family;
			if(memcmp(&sas, &sas_zero, sizeof(sas)) != 0){
				bound_addr = unique_ptr<NetAddress>(new NetAddress(sas));
			}
		}
	}
	return bound_addr;
}

Socket::~Socket(){
	if(fd >= 0) close(fd);
}

#if defined __WIN32
#include <Winsock2.h>
void Socket::setNonBlocking(bool enabled){
	u_long l = enabled ? 1 : 0;
	ioctlsocket(fd, FIONBIO, &i);
}
#else
#include <fcntl.h>
void Socket::setNonBlocking(bool enabled){
	int flags = fcntl(fd, F_GETFL, 0);
	flags = (enabled ? flags | O_NONBLOCK : flags & ~O_NONBLOCK);
	fcntl(fd, F_SETFL, flags);
}
#endif

void Socket::enableTimestamps(bool enable){
	do_timestamp = enable;
}

#include <err.h>
#include <netinet/ip.h>

UDPSocket::UDPSocket(int family)
: Socket(SOCK_DGRAM, family){

}

UDPSocket::UDPSocket(const NetAddress& a)
: Socket(SOCK_DGRAM, a){

}



