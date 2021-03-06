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
#include "nrg_connection.h"
#include "nrg_packet_header.h"
#include "nrg_config.h"
#include "nrg_os.h"
#include <climits>
#include <cstdlib>
#include <cassert>

using namespace nrg;
using namespace std;

ConnectionCommon::ConnectionCommon()
: seq_num(os::random())
, transform(NULL) {

}

void ConnectionCommon::setTransform(PacketTransformation* t){
	transform = t;
}

ConnectionIn::ConnectionIn()
: new_packet(false)
, full_packet(false)
, latest(NRG_MAX_PACKET_SIZE) {

}

bool ConnectionIn::addPacket(Packet& p){
	PacketHeader h;

	if(cc.transform){
		if(!cc.transform->remove(p, buffer.reset())){
			return false; // removal of packet transformation failed.
		}
	}

	Packet& input = cc.transform ? buffer : p;

	if(!h.read(input) || h.version != 0){
		return false; // packet has invalid header / unknown version.
	}

	int packet_age = 0;

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

		if(seq_dist > 0){ // new packet
			packet_history <<= seq_dist;
			packet_history[0] = 1;
			cc.seq_num = h.seq_num;

			// update ages of reassembly fragments and remove them if they're too old.
			for(auto& i : reassembly_buf){
				if(i.age >= 0) i.age += seq_dist;
				if(i.age > (int)NRG_CONN_PACKET_HISTORY){
					i.age = -1;
					i.data.reset();
				}
			}
		} else {
			if(packet_history[-seq_dist]){ // already recieved this packet.
				return true;
			} else {
				packet_history[-seq_dist] = 1;
				packet_age = -seq_dist;
			}
		}
	}

	if((h.flags & PKTFLAG_FINISHED) && (cc.seq_num != h.seq_num)){
		return false; // packet claims end of stream, but there is a newer one...
	}

	new_packet = true;

	// reconstruct packets that were split into multiple fragments.
	if(h.frag_index > 0 || (h.flags & PKTFLAG_CONTINUED)){

		if(h.frag_index >= reassembly_buf.size()){
			return false;
		}

		ReassemblyInfo& frag = reassembly_buf[h.frag_index];

		if(frag.age >= 0 && packet_age > frag.age){
			new_packet = false;
		} else {
			frag.data.writeArray(input.getPointer(), input.remaining());
			frag.age = packet_age;
			frag.continued = h.flags & PKTFLAG_CONTINUED;

			// check if the newly added fragment completes a full packet.
			for(size_t i = 0; i < reassembly_buf.size(); ++i){
				NRG_DEBUG("Age calc: [%d, %d]\n", (int)i, reassembly_buf[i].age);
				if(reassembly_buf[i].age == -1
				|| (i > 0 && reassembly_buf[i].age != reassembly_buf[i-1].age - 1)){
					new_packet = false;
				}
				if(!reassembly_buf[i].continued) break;
			}
			NRG_DEBUG("Got fragment, idx: %d, continued?: %d, age: %d full?: %d\n",
				h.frag_index, h.flags & PKTFLAG_CONTINUED, packet_age, new_packet
			);
			// write all the fragments out if they do form a full packet.
			if(new_packet){
				latest.reset();

				int age_counter = reassembly_buf[0].age;
				for(auto& i : reassembly_buf){
					if(i.age != age_counter--) break;
					latest.writeArray(i.data.getBasePointer(), i.data.size());
					i.data.reset();
					i.age = -1;
				}
			}
		}
	} else {
		latest.reset().writeArray(input.getPointer(), input.remaining());
	}

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
: remote_addr(na)
, sock(sock)
, last_status(true) {

}

Status ConnectionOut::sendPacket(Packet& p, PacketFlags f){
	off_t o = p.tell();
	p.seek(0, SEEK_SET);

	last.reset().writeArray(p.getBasePointer(), p.size());
	last_header = PacketHeader(cc.seq_num++, f & ~PKTFLAG_OUT_OF_ORDER);

	sendPacketWithHeader(p, last_header);

	p.seek(o, SEEK_SET);

	return last_status;
}

Status ConnectionOut::sendPacketWithHeader(Packet& p, PacketHeader h){
	assert(&p != &buffer);

	do {
		size_t n = std::min(p.remaining(), NRG_MAX_PACKET_SIZE - PacketHeader::size);

		if(p.remaining() > NRG_MAX_PACKET_SIZE - PacketHeader::size){
			h.flags |= PKTFLAG_CONTINUED;
		}

		h.write(buffer.reset());
		buffer.writeArray(p.getPointer(), n);

		if(cc.transform) cc.transform->apply(buffer, buffer2.reset());

		Packet& sendme = cc.transform ? buffer2 : buffer;
		last_status = sock.sendPacket(sendme, remote_addr);
		if(!last_status){
			return last_status;
		}

		p.seek(n, SEEK_CUR);
		++h.frag_index;

	} while(p.remaining());

	return last_status = StatusOK();
}

Status ConnectionOut::resendLastPacket(void){
	PacketHeader h(last_header);
	h.flags |= PKTFLAG_RETRANSMISSION;

	return sendPacketWithHeader(last.seek(0, SEEK_SET), h);
}

Status ConnectionOut::sendDisconnect(Packet& p){
	off_t o = p.tell();
	p.seek(0, SEEK_SET);

	last.reset().writeArray(p.getPointer(), p.remaining());
	last_header = PacketHeader(cc.seq_num++, PKTFLAG_FINISHED);

	sendPacketWithHeader(last.seek(0, SEEK_SET), last_header);

	p.seek(o, SEEK_SET);

	return last_status;
}

Status ConnectionOut::getLastStatus() const {
	return last_status;
}
