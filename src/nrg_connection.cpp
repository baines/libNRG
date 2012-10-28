#include "nrg_connection.h"
#include "nrg_config.h"
#include <climits>

nrg::ConnectionBase::ConnectionBase(const NetAddress& na) : remote_addr(na) {

};

nrg::ConnectionIncoming::ConnectionIncoming(const NetAddress& na)
: ConnectionBase(na), latest(NRG_MAX_PACKET_SIZE), partial(NRG_MAX_PACKET_SIZE) {

}

bool nrg::ConnectionIncoming::isValidPacketHeader(uint16_t seq, uint8_t flags){
	bool valid = false;
		
	if(flags & PKTFLAG_CONTINUATION){
		valid = (seq == (seq_num + 1));
	} else {
		for(int i = 1; i < NRG_NUM_PAST_STATES; ++i){
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
		
		if(!isValidPacketHeader(seq, flags)){
			p.seek(o, SEEK_SET);
			return false;
		}
		
		if(flags & PKTFLAG_CONTINUATION){
			partial.writeArray(p.getPointer(), p.remaining());
			if(!(flags & PKTFLAG_CONTINUED)) partial.markComplete();
		} else {
			Packet& ref = (flags & PKTFLAG_CONTINUED) ? partial : latest;
			ref.reset();
			p.seek(0, SEEK_SET);
			ref.writeArray(p.getPointer(), p.remaining());
		}

		p.seek(o, SEEK_SET);
		return true;
	} else {
		return false;
	}
}
