/*
  LibNRG - Networking for Real-time Games

  Copyright (C) 2012-2014 Alex Baines <alex@abaines.me.uk>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include "nrg_socket.h"
#include "nrg_config.h"
#include "nrg_os.h"
#ifdef NRG_ENABLE_MSG_ERRQUEUE
    #include <linux/errqueue.h>
#endif
#ifdef _WIN32
	#undef errno
	#define errno WSAGetLastError()
#endif

using namespace nrg;
using std::unique_ptr;

Socket::Socket(int type, int family)
: bound_addr()
, connected_addr()
, fd(0)
, family(family)
, type(type)
, do_timestamp(false)
, use_errqueue(false)
, last_timestamp(0) {
	if(family != PF_UNSPEC) fd = socket(family, type, 0);
}

Socket::Socket(int type, const NetAddress& a)
: bound_addr()
, connected_addr()
, fd(0)
, family(a.getFamily())
, type(type)
, do_timestamp(false)
, use_errqueue(false)
, last_timestamp(0) {
	fd = socket(family, type, 0);
}

void Socket::setFamilyFromAddress(const NetAddress& na){
	family = na.getFamily();
	fd = socket(family, type, 0);
}

Status Socket::bind(const NetAddress& addr){
	if(!fd){
		family = addr.getFamily();
		fd = socket(family, type, 0);
	} else if(addr.getFamily() != family){
		return Status("Socket is set to a different family than the address.");
	}

	socklen_t len = 0;
	const struct sockaddr* sa = addr.toSockAddr(len);
	if(::bind(fd, sa, len) == 0){
		bound_addr = unique_ptr<NetAddress>(new NetAddress(addr));
		return StatusOK();
	} else {
		return StatusErr(errno);
	}
}

Status Socket::connect(const NetAddress& addr){
	if(!fd){
		family = addr.getFamily();
		fd = socket(family, type, 0);
	} else if(addr.getFamily() != family){
		return Status("Socket is set to a different family than the address.");
	}

	socklen_t len = 0;
	const struct sockaddr* sa = addr.toSockAddr(len);
	if(::connect(fd, sa, len) == 0){
		connected_addr = unique_ptr<NetAddress>(new NetAddress(addr));
		return StatusOK();
	} else {
		return StatusErr(errno);
	}
}

void Socket::disconnect(){
	connected_addr = nullptr;
}

bool Socket::isConnected() const {
	return bool(connected_addr);
}

Status Socket::sendPacket(const Packet& p) const {
	size_t sz = 0, attempts = 0;

	// send() should send a full datagram, but loop just in case.
	do {
		const char* ptr = reinterpret_cast<const char*>(p.getBasePointer() + sz);
		ssize_t res = ::send(fd, ptr, p.size() - sz, 0);
		if(res == -1){
			int err = errno;
			if(attempts++ < 3 && (err == EAGAIN || err == EWOULDBLOCK)){
				continue;
			} else {
				return StatusErr(err);
			}
		} else {
			sz = std::max(sz + res, p.size());
		}
	} while(sz < p.size());

	return StatusOK();
}

Status Socket::sendPacket(const Packet& p, const NetAddress& addr) const {
	size_t sz = 0, attempts = 0;
	socklen_t len = 0;
	const struct sockaddr* sa = addr.toSockAddr(len);

	// sendto() should send a full datagram, but loop just in case.
	do {
		const char* ptr = reinterpret_cast<const char*>(p.getBasePointer() + sz);
		ssize_t res = ::sendto(fd, ptr, p.size() - sz, 0, sa, len);
		if(res == -1){
			int err = errno;
			if(attempts++ < 3 && (err == EAGAIN || err == EWOULDBLOCK)){
				continue;
			} else {
				return StatusErr(err);
			}
		} else {
			sz = std::max(sz + res, p.size());
		}
	} while(sz < p.size());

	return StatusOK();
}

Status Socket::recvPacket(Packet& p) const {
	char buf[NRG_MAX_PACKET_SIZE];

	ssize_t result = ::recv(fd, buf, NRG_MAX_PACKET_SIZE, 0);

	if(result > 0){
		p.writeArray(buf, result);
		return StatusOK();
	} else {
		return StatusErr(errno);
	}
}

Status Socket::recvPacket(Packet& p, NetAddress& addr) {
	struct sockaddr_storage sas = {};
	char buf[NRG_MAX_PACKET_SIZE];
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
	addr = sas;
	
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
		return StatusOK();
	} else {
		int err = errno;
		if(use_errqueue){
			addr = NetAddress();
			return checkErrorQueue(addr);
		} else {
			printf("SOCKET ERROR [%s:%d] [%d].\n", addr.getIP(), addr.getPort(), err);
			return StatusErr(err);
		}
	}
}

// Linux/POSIX specific
Status Socket::checkErrorQueue(NetAddress& culprit){
#ifdef NRG_ENABLE_MSG_ERRQUEUE
	struct sockaddr_storage sas = {};
	uint8_t buf[NRG_MAX_PACKET_SIZE];
	socklen_t len = sizeof(sas);

	struct msghdr msg = {};
    struct iovec iov = {};

	char cbuf[512];
	struct cmsghdr* cmsg = (struct cmsghdr*)cbuf;

	iov.iov_base = buf;
	iov.iov_len = sizeof(buf);

	msg.msg_name = &sas;
	msg.msg_namelen = len;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control = cmsg;
	msg.msg_controllen = sizeof(cbuf);

	ssize_t result = ::recvmsg(fd, &msg, MSG_ERRQUEUE);

	if(result < 0) return StatusErr(errno);

	Status ret = StatusOK();
	for(cmsg = CMSG_FIRSTHDR(&msg); cmsg; cmsg = CMSG_NXTHDR(&msg, cmsg)){
		if(cmsg->cmsg_level == IPPROTO_IP && cmsg->cmsg_type == IP_RECVERR){
			struct sock_extended_err* err = (struct sock_extended_err*) CMSG_DATA(cmsg);
			// For some reason the port in this addr is always 0, but msg.msg_name has the correct one.
			//struct sockaddr* sa = SO_EE_OFFENDER(err);

			if(sas.ss_family != AF_UNSPEC){
				culprit = sas;
				printf("MSG_ERRQUEUE: [%s:%d]. [%d]\n", culprit.getIP(), culprit.getPort(), err->ee_errno);
			}
			ret = StatusErr(err->ee_errno);
		}
	}

	return ret;
#else
	return StatusOK();
#endif
}

bool Socket::dataPending(int usToBlock) const {
	fd_set s;
	FD_ZERO(&s);
	FD_SET(fd, &s);
	struct timeval tv = { 0, usToBlock };

	if(select(fd+1, &s, NULL, NULL, &tv) == 1){
		return FD_ISSET(fd, &s);
	} else {
		return false;
	}
}

const std::unique_ptr<NetAddress>& Socket::getBoundAddress() const {
	if(!bound_addr){
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
	if(fd >= 0){
#ifdef _WIN32
		closesocket(fd);
#else
		close(fd);
#endif
	}
}

#if defined __WIN32
void Socket::setNonBlocking(bool enabled){
	u_long l = enabled ? 1 : 0;
	ioctlsocket(fd, FIONBIO, &l);
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

void Socket::handleUnconnectedICMPErrors(bool enable){
#if defined(_WIN32)
	/* Seems to work even without this setting */ 
	//int i_enable = enable;
	//WSAIoctl(fd, SIO_UDP_CONNRESET, &i_enable, sizeof(i_enable), nullptr, 0, nullptr, nullptr, nullptr);
#elif defined(__linux) && defined(NRG_ENABLE_MSG_ERRQUEUE)
	use_errqueue = enable;
	setOption<int>(IPPROTO_IP, IP_RECVERR, enable);
#endif
}

UDPSocket::UDPSocket(int family)
: Socket(SOCK_DGRAM, family){

}

UDPSocket::UDPSocket(const NetAddress& a)
: Socket(SOCK_DGRAM, a){

}



