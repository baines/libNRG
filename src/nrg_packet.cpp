#include "nrg_packet.h"

nrg::Packet::Packet(size_t initial_size) 
: data(new uint8_t[initial_size]), pointer(data), data_size(initial_size) {

}

nrg::Packet::~Packet(){
	delete [] data;
}

nrg::Packet& nrg::Packet::write8(uint8_t v){
	if(pointer - data <= (data_size - sizeof(v))){
		*pointer++ = v;
	}
	return *this;
}

nrg::Packet& nrg::Packet::write16(uint16_t v){
	return writeBE(htons(v));
}

nrg::Packet& nrg::Packet::write32(uint32_t v){
	return writeBE(htonl(v));
}
