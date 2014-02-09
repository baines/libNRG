#ifndef NRG_PACKET_HEADER_H
#define NRG_PACKET_HEADER_H

namespace nrg {

struct PacketHeader {
	PacketHeader()
	: seq_num(0)
	, flags(0)
	, version(0)
	, size(4)
	, frag_index(0){
	
	}
	
	PacketHeader(uint16_t seq, uint8_t flags, uint8_t frag_index = 0)
	: seq_num(seq)
	, flags(flags)
	, version(0)
	, size(4)
	, frag_index(0){
	
	}

	bool valid();
	bool read(Packet& p){
		if(p.remaining() >= size){
			uint8_t ver_idx = 0;
			p.read8(ver_idx).read8(flags).read16(seq_num);
			
			version = ver_idx >> 6;
			frag_index = ver_idx & 0x3F;
			return true;
		} else {
			return false;
		}
	}
	void write(Packet& p){
		p.write8((version << 6) | (frag_index & 0x3F));
		p.write8(flags);
		p.write16(seq_num);
	}

	uint16_t seq_num;
	uint8_t flags;
	uint32_t version, size, frag_index;
};

}

#endif
