#include "nrg_connection.h"
#include "nrg_config.h"
#include "nrg_os.h"
#include <climits>
#include <cstdlib>
#include <cassert>
#include <iostream>

using namespace nrg;
using namespace std;

ConnectionCommon::ConnectionCommon(const NetAddress& na) : remote_addr(na), 
seq_num(os::random()), transform(NULL) {

}

void ConnectionCommon::setTransform(PacketTransformation* t){
	transform = t;
}

ConnectionIn::ConnectionIn(const NetAddress& na)
: cc(na), new_packet(false), first_packet(true), full_packet(false), 
  latest(NRG_MAX_PACKET_SIZE) {

}

bool ConnectionIn::isValidPacketHeader(uint16_t seq, uint8_t flags) {
	bool valid = false;
	
	if(flags & PKTFLAG_CONTINUATION){
		if(latest.tell() != 0 && !full_packet){
			valid = seq == (cc.seq_num + 1);
		}
	} else {
		valid = (seq - cc.seq_num) < NRG_NUM_PAST_SNAPSHOTS;
	}
	return valid;
}

bool ConnectionIn::addPacket(Packet& p){
	uint16_t seq = 0;
	uint8_t flags = 0;
	
	buffer.reset();
	p.seek(0, SEEK_SET);
	if(cc.transform && !cc.transform->remove(p, buffer)) return false;
	Packet& ref = cc.transform ? buffer : p;
	
	if(ref.size() >= (sizeof(seq) + sizeof(flags))){
		off_t o = p.tell();
		ref.seek(0, SEEK_SET);
		ref.read16(seq).read8(flags);
				
		if(first_packet){
			if(flags & PKTFLAG_CONTINUATION){
				ref.seek(o, SEEK_SET);
				return false;
			} else {
				first_packet = false;
			}
		} else if(!isValidPacketHeader(seq, flags)){ //XXX near out of order are valid
			ref.seek(o, SEEK_SET);
			return true;
		}

		if(flags & PKTFLAG_FINISHED){
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
		
		latest_flags = static_cast<PacketFlags>(flags); // XXX PKTFLAG_OUT_OF_ORDER
		cc.seq_num = seq;
		ref.seek(o, SEEK_SET);
		return true;
	} else {
		return false;
	}
}

bool ConnectionIn::hasNewPacket() const {
	return new_packet;
}

PacketFlags ConnectionIn::getLatestPacket(Packet& p){
	p.writeArray(latest.getBasePointer(), latest.size());
	p.seek(0, SEEK_SET);
	new_packet = false;
	
	return latest_flags;
}

ConnectionOut::ConnectionOut(const NetAddress& na, const Socket& sock)
: cc(na), sock(sock) {
}

void ConnectionOut::sendPacket(Packet& p, PacketFlags f){
	uint8_t user_flags = f & (PKTFLAG_STATE_CHANGE | PKTFLAG_STATE_CHANGE_ACK);
	assert(user_flags == f);
	
	buffer.reset();
	uint8_t flags = 0;
	off_t o = p.tell();
	p.seek(0, SEEK_SET);	

	do {
		size_t n = std::min(p.remaining(), NRG_MAX_PACKET_SIZE - cc.getHeaderSize());

		if(p.remaining() > NRG_MAX_PACKET_SIZE - cc.getHeaderSize()){
			flags |= PKTFLAG_CONTINUED;
		}
		
		buffer.write16(cc.seq_num++).write8(flags | user_flags);
		assert(*(buffer.getPointer()-1) == (flags | user_flags));
		buffer.writeArray(p.getPointer(), n);
		p.seek(n, SEEK_CUR);
		buffer.seek(0, SEEK_SET);
		if(cc.transform){
			cc.transform->apply(buffer, buffer2.reset());
			sock.sendPacket(buffer2, cc.remote_addr);
		} else {
			sock.sendPacket(buffer, cc.remote_addr);
		}
		flags = PKTFLAG_CONTINUATION;
	} while(p.remaining());
	p.seek(o, SEEK_SET);
}

bool ConnectionOut::resendLastPacket(void){
	if(buffer.size() <= sizeof(cc.seq_num)){ //XXX separate Packet for last packet since may be split
		return false;
	}
	
	uint8_t old_flags = 0;
	
	buffer.seek(sizeof(cc.seq_num), SEEK_SET);
	buffer.read8(old_flags);
	buffer.seek(-1, SEEK_CUR);
	buffer.write8(old_flags | PKTFLAG_RETRANSMISSION);
	    	
	if(cc.transform){
		cc.transform->apply(buffer, buffer2.reset());
		return sock.sendPacket(buffer2, cc.remote_addr) > 0;
	} else {
		return sock.sendPacket(buffer, cc.remote_addr) > 0;
	}
}

void ConnectionOut::sendDisconnect(Packet& p){
	off_t o = p.tell();
	p.seek(0, SEEK_SET);
	size_t n = std::min(p.remaining(), NRG_MAX_PACKET_SIZE - cc.getHeaderSize());

	buffer.reset().write16(cc.seq_num++).write8(PKTFLAG_FINISHED).writeArray(p.getPointer(), n);
	buffer.seek(0, SEEK_SET);
	
	if(cc.transform){
		cc.transform->apply(buffer, buffer2.reset());
		sock.sendPacket(buffer2, cc.remote_addr);
	} else {
		sock.sendPacket(buffer, cc.remote_addr);
	}

	p.seek(o, SEEK_SET);
}
