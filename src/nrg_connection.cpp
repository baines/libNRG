#include "nrg_connection.h"
#include "nrg_packet_header.h"
#include "nrg_config.h"
#include "nrg_os.h"
#include <climits>
#include <cstdlib>

using namespace nrg;
using namespace std;

ConnectionCommon::ConnectionCommon(const NetAddress& na) 
: remote_addr(na)
, seq_num(os::random())
, transform(NULL) {

}

void ConnectionCommon::setTransform(PacketTransformation* t){
	transform = t;
}

ConnectionIn::ConnectionIn(const NetAddress& na)
: cc(na)
, new_packet(false)
, full_packet(false)
, latest(NRG_MAX_PACKET_SIZE) {

}

bool ConnectionIn::addPacket(Packet& p){
	PacketHeader h;
	
	if(cc.transform){
		if(!cc.transform->remove(p, buffer.reset())){
			return false; // removal of packet transformation failed.
		}
	} else {
		buffer = p;
	}
	
	if(!h.read(buffer) || h.version != 0){
		return false; // packet has invalid header / unknown version.
	}
	
	if(packet_history.none()){ // first packet recieved
		cc.seq_num = h.seq_num;
		packet_history[0] = 1;
	} else {
		int16_t seq_dist = static_cast<int16_t>(h.seq_num - cc.seq_num);
		
		if(abs(seq_dist) >= NRG_CONN_PACKET_HISTORY){
			return true; // packet is out of history range.
		}
		
		// states need to know about retransmissons of the newest packet
		if(seq_dist < 0 && (h.flags & PKTFLAG_RETRANSMISSION)){
			return true;
		} 
		
		if(seq_dist > 0){
			packet_history << seq_dist;
			packet_history[0] = 1;
			cc.seq_num = h.seq_num;
		} else {
			if(packet_history[-seq_dist]){ // already recieved this packet.
				return true;
			} else {
				packet_history[-seq_dist] = 1;
			}
		}
	}
	
	if((h.flags & PKTFLAG_FINISHED) && (cc.seq_num != h.seq_num)){
		return false; // packet claims end of stream, but there is a newer one...
	}
	
	// do reconstruction here
	
	new_packet = true;
	latest.reset().writeArray(buffer.getPointer(), buffer.remaining());
	if(cc.seq_num > h.seq_num) h.flags |= PKTFLAG_OUT_OF_ORDER;
	latest_flags = static_cast<PacketFlags>(h.flags);
	
	return true;
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
: cc(na)
, sock(sock)
, last_status(true) {

}

Status ConnectionOut::sendPacket(Packet& p, PacketFlags f){
	uint8_t user_flags = f & (PKTFLAG_STATE_CHANGE | PKTFLAG_STATE_CHANGE_ACK);
	uint8_t frag_index = 0;
	
	buffer.reset();
	uint8_t flags = 0;
	off_t o = p.tell();
	p.seek(0, SEEK_SET);	

	do {
		size_t n = std::min(p.remaining(), NRG_MAX_PACKET_SIZE - cc.getHeaderSize());

		if(p.remaining() > NRG_MAX_PACKET_SIZE - cc.getHeaderSize()){
			flags |= PKTFLAG_CONTINUED;
		}
		
		PacketHeader(cc.seq_num++, flags | user_flags, frag_index).write(buffer);
		
		buffer.writeArray(p.getPointer(), n);
		p.seek(n, SEEK_CUR);
		buffer.seek(0, SEEK_SET);
		
		Packet& sendme = cc.transform ? buffer2 : buffer;
		if(cc.transform) cc.transform->apply(buffer, buffer2.reset());
		if(!(last_status = sock.sendPacket(sendme, cc.remote_addr))) return last_status;
		
		++frag_index;
		
	} while(p.remaining());
	
	p.seek(o, SEEK_SET);

	return last_status = StatusOK();
}

Status ConnectionOut::resendLastPacket(void){ //XXX consider split packets
	PacketHeader h;

	buffer.seek(0, SEEK_SET);
	if(!h.read(buffer)){
		return last_status = Status("Can't read last packet's header.");
	}
	h.flags |= PKTFLAG_RETRANSMISSION;
	
	buffer.seek(0, SEEK_SET);
	h.write(buffer);
	    	
	if(cc.transform){
		cc.transform->apply(buffer, buffer2.reset());
		return (last_status = sock.sendPacket(buffer2, cc.remote_addr));
	} else {
		return (last_status = sock.sendPacket(buffer, cc.remote_addr));
	}
}

Status ConnectionOut::sendDisconnect(Packet& p){
	off_t o = p.tell();
	p.seek(0, SEEK_SET);
	size_t n = std::min(p.remaining(), NRG_MAX_PACKET_SIZE - cc.getHeaderSize());

	buffer.reset();
	PacketHeader(cc.seq_num++, PKTFLAG_FINISHED).write(buffer);
	buffer.writeArray(p.getPointer(), n);
	buffer.seek(0, SEEK_SET);
	p.seek(o, SEEK_SET);
		
	if(cc.transform){
		cc.transform->apply(buffer, buffer2.reset());
		return (last_status = sock.sendPacket(buffer2, cc.remote_addr));
	} else {
		return (last_status = sock.sendPacket(buffer, cc.remote_addr));
	}
}

Status ConnectionOut::getLastStatus() const {
	return last_status;
}
