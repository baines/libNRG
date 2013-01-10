#include "nrg_connection.h"
#include "nrg_config.h"
#include "nrg_os.h"
#include <climits>
#include <cstdlib>

nrg::ConnectionBase::ConnectionBase(const NetAddress& na) : remote_addr(na), 
seq_num(os::random()) {

};

nrg::ConnectionIncoming::ConnectionIncoming(const NetAddress& na)
: ConnectionBase(na), new_packet(false), first_packet(true), full_packet(false), 
  latest(NRG_MAX_PACKET_SIZE) {

}

bool nrg::ConnectionIncoming::isValidPacketHeader(uint16_t seq, uint8_t flags) const {
	bool valid = false;
	
	if(flags & PKTFLAG_CONTINUATION){
		if(latest.tell() != 0 && !full_packet){
			valid = (seq == ((seq_num + 1) & USHRT_MAX));
		}
	} else {
		for(int i = 1; i < NRG_NUM_PAST_SNAPSHOTS; ++i){
			uint16_t v = (seq_num + i) & USHRT_MAX;
			if(v == seq){
				valid = true;
				break;
			}
		}
	}
	return valid;
}

bool nrg::ConnectionIncoming::addPacket(Packet& p){
	uint16_t seq = 0;
	uint8_t flags = 0;
	
	if(p.size() >= (sizeof(seq) + sizeof(flags))){
		off_t o = p.tell();
		p.seek(0, SEEK_SET).read16(seq).read8(flags);
				
		if(first_packet){
			if(flags & PKTFLAG_CONTINUATION){
				p.seek(o, SEEK_SET);
				return false;
			} else {
				first_packet = false;
			}
		} else if(!isValidPacketHeader(seq, flags)){
			p.seek(o, SEEK_SET);
			return false;
		}
		
		if(!(flags & PKTFLAG_CONTINUATION)){
			latest.reset();
			full_packet = false;
		}
		
		latest.writeArray(p.getPointer(), p.remaining());
		
		if(!(flags & PKTFLAG_CONTINUED)){
			full_packet = true;
			new_packet = true;
		}
		
		seq_num = seq;
		p.seek(o, SEEK_SET);
		return true;
	} else {
		return false;
	}
}

bool nrg::ConnectionIncoming::hasNewPacket() const {
	return new_packet;
}

void nrg::ConnectionIncoming::getLatestPacket(Packet& p){
	off_t o = latest.tell();
	latest.seek(0, SEEK_SET);
	p.writeArray(latest.getPointer(), latest.remaining());
	p.seek(0, SEEK_SET);
	latest.seek(o, SEEK_SET);
	new_packet = false;
}

nrg::ConnectionOutgoing::ConnectionOutgoing(const NetAddress& na, const Socket& sock)
: ConnectionBase(na), sock(sock) {
}

void nrg::ConnectionOutgoing::sendPacket(Packet& p){
	Packet p2(NRG_MAX_PACKET_SIZE);
	uint8_t flags = 0;
	off_t o = p.tell();
	p.seek(0, SEEK_SET);	

	while(p.remaining()){
		size_t n = std::min(p.remaining(), NRG_MAX_PACKET_SIZE - getHeaderSize());

		if(p.remaining() > NRG_MAX_PACKET_SIZE - getHeaderSize()){
			flags |= PKTFLAG_CONTINUED;
		}

		p2.reset().write16(seq_num++).write8(flags).writeArray(p.getPointer(), n);
		p.seek(n, SEEK_CUR);
		sock.sendPacket(p2, remote_addr);
		flags = PKTFLAG_CONTINUATION;
	}
	p.seek(o, SEEK_SET);
}
