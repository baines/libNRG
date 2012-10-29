#include "nrg_packet.h"
#include <algorithm>

nrg::Packet::Packet(size_t initial_size) 
: data(new uint8_t[initial_size]), pointer(data), data_size(initial_size)
, used_size(0) {

}

nrg::Packet::~Packet(){
	delete [] data;
}

nrg::Packet& nrg::Packet::write8(const uint8_t& v){
	return writeBE(v);
}

nrg::Packet& nrg::Packet::write16(const uint16_t& v){
	return writeBE(htons(v));
}

nrg::Packet& nrg::Packet::write32(const uint32_t& v){
	return writeBE(htonl(v));
}

nrg::Packet& nrg::Packet::writeArray(const uint8_t* v, size_t size){
	while((pointer - data) > (data_size - size)){
		resize();
	}
	memcpy(pointer, v, size);
	pointer += size;
	used_size += size;
	return *this;
}

nrg::Packet& nrg::Packet::read8(uint8_t& v){
	if(pointer - data <= (data_size - sizeof(v))){
		v = *pointer++;
	}
	return *this;
}

nrg::Packet& nrg::Packet::read16(uint16_t& v){
	uint16_t be_v;
	readBE(be_v);
	v = ntohs(be_v);
	return *this;
}

nrg::Packet& nrg::Packet::read32(uint32_t& v){
	uint32_t be_v;
	readBE(be_v);
	v = ntohl(be_v);
	return *this;
}

nrg::Packet& nrg::Packet::reset(){
	memset(data, 0, data_size);
	used_size = 0;
	pointer = data;
}

nrg::Packet& nrg::Packet::seek(off_t offset, int whence){
	switch(whence){
	case SEEK_CUR:
		pointer += offset; break;
	case SEEK_SET:
		pointer = data + offset; break;
	case SEEK_END:
		pointer = (data + data_size) - offset; break;
	}
	pointer = std::max(data, std::min(data + data_size, pointer));
	return *this;
}

off_t nrg::Packet::tell() const {
	return pointer - data;
}

void nrg::Packet::resize(){
	uint8_t* new_data = new uint8_t[data_size * 2];
	memcpy(new_data, data, data_size);
	data_size *= 2;
	pointer = new_data + (pointer - data);
	delete [] data;
	data = new_data;
}

