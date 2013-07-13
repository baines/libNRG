#include "nrg_connection.h"
#include "nrg_config.h"
#include "nrg_os.h"
#include <climits>
#include <cstdlib>

nrg::ConnectionBase::ConnectionBase(const NetAddress& na) : remote_addr(na), 
seq_num(os::random()), transform(NULL) {

};

void nrg::ConnectionBase::setTransform(nrg::PacketTransformation* t){
	transform = t;
}

nrg::ConnectionIncoming::ConnectionIncoming(const NetAddress& na)
: ConnectionBase(na), new_packet(false), first_packet(true), full_packet(false), 
  final_packet(false), latest(NRG_MAX_PACKET_SIZE) {

}

bool nrg::ConnectionIncoming::isValidPacketHeader(uint16_t seq, uint8_t flags) const {
	bool valid = false;
	
	if(flags & PKTFLAG_CONTINUATION){
		if(latest.tell() != 0 && !full_packet){
			valid = seq == (seq_num + 1);
		}
	} else {
		valid = (seq - seq_num) < NRG_NUM_PAST_SNAPSHOTS;
	}
	return valid;
}

bool nrg::ConnectionIncoming::addPacket(Packet& p){
	if(final_packet) return false;
	
	uint16_t seq = 0;
	uint8_t flags = 0;
	
	buffer.reset();
	p.seek(0, SEEK_SET);
	if(transform && !transform->remove(p, buffer)) return false;
	Packet& ref = transform ? buffer : p;
	
	if(ref.size() >= (sizeof(seq) + sizeof(flags))){
		off_t o = p.tell();
		ref.seek(0, SEEK_SET).read16(seq).read8(flags);
				
		if(first_packet){
			if(flags & PKTFLAG_CONTINUATION){
				ref.seek(o, SEEK_SET);
				return false;
			} else {
				first_packet = false;
			}
		} else if(!isValidPacketHeader(seq, flags)){
			ref.seek(o, SEEK_SET);
			return true;
		}

		if(flags & PKTFLAG_FINISHED){
			final_packet = true;
			flags &= ~(PKTFLAG_CONTINUATION | PKTFLAG_CONTINUED);
		}
		
		if(!(flags & PKTFLAG_CONTINUATION)){
			latest.reset();
		}
		
		latest.writeArray(ref.getPointer(), ref.remaining());
		
		if(!(flags & PKTFLAG_CONTINUED)){
			full_packet = true;
			new_packet = true;
		} else {
			full_packet = false;
		}
		
		seq_num = seq;
		ref.seek(o, SEEK_SET);
		return true;
	} else {
		return false;
	}
}

bool nrg::ConnectionIncoming::hasNewPacket() const {
	return new_packet;
}

void nrg::ConnectionIncoming::getLatestPacket(Packet& p){
	p.writeArray(latest.getBasePointer(), latest.size()).seek(0, SEEK_SET);
	new_packet = false;
}

bool nrg::ConnectionIncoming::isLatestPacketFinal() const {
	return final_packet;
}

nrg::ConnectionOutgoing::ConnectionOutgoing(const NetAddress& na, const Socket& sock)
: ConnectionBase(na), sock(sock) {
}

void nrg::ConnectionOutgoing::sendPacket(Packet& p){
	buffer.reset();
	uint8_t flags = 0;
	off_t o = p.tell();
	p.seek(0, SEEK_SET);	

	while(p.remaining()){
		size_t n = std::min(p.remaining(), NRG_MAX_PACKET_SIZE - getHeaderSize());

		if(p.remaining() > NRG_MAX_PACKET_SIZE - getHeaderSize()){
			flags |= PKTFLAG_CONTINUED;
		}

		buffer.write16(seq_num++).write8(flags).writeArray(p.getPointer(), n);
		p.seek(n, SEEK_CUR);
		buffer.seek(0, SEEK_SET);
		if(transform){
			transform->apply(buffer, buffer2.reset());
			sock.sendPacket(buffer2, remote_addr);
		} else {
			sock.sendPacket(buffer, remote_addr);
		}
		flags = PKTFLAG_CONTINUATION;
	}
	p.seek(o, SEEK_SET);
}

void nrg::ConnectionOutgoing::sendDisconnect(Packet& p){
	off_t o = p.tell();
	p.seek(0, SEEK_SET);
	size_t n = std::min(p.remaining(), NRG_MAX_PACKET_SIZE - getHeaderSize());

	buffer.reset().write16(seq_num++).write8(PKTFLAG_FINISHED)
	      .writeArray(p.getPointer(), n).seek(0, SEEK_SET);
	
	if(transform){
		transform->apply(buffer, buffer2.reset());
		sock.sendPacket(buffer2, remote_addr);
	} else {
		sock.sendPacket(buffer, remote_addr);
	}

	p.seek(o, SEEK_SET);
}
