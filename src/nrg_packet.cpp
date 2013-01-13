#include "nrg_packet.h"
#include "nrg_config.h"
#include <algorithm>

nrg::Packet::Packet() : data(new uint8_t[NRG_MAX_PACKET_SIZE]), pointer(data), 
data_size(NRG_MAX_PACKET_SIZE), used_size(0) {

}

nrg::Packet::Packet(size_t initial_size) 
: data(new uint8_t[initial_size]), pointer(data), data_size(initial_size)
, used_size(0) {

}

nrg::Packet::Packet(const Packet& copy) : data(new uint8_t[copy.data_size]), 
pointer(data + (copy.pointer - copy.data)), data_size(copy.data_size), 
used_size(copy.used_size){
	memcpy(data, copy.data, copy.used_size);
}

nrg::Packet& nrg::Packet::operator=(const nrg::Packet& other){
	reset();
	while(data_size < other.used_size) resize();
	used_size = other.used_size;
	memcpy(data, other.data, other.used_size);
	pointer = data + (other.pointer - other.data);
	return *this;
}

nrg::Packet::~Packet(){
	if(data) delete [] data;
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
	used_size = std::max(used_size, (size_t)(pointer - data));
	return *this;
}

nrg::Packet& nrg::Packet::read8(uint8_t& v){
	if(pointer - data <= (used_size - sizeof(v))){
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

nrg::Packet& nrg::Packet::readArray(uint8_t* v, size_t size){
	if(pointer - data <= (used_size - size)){
		memcpy(v, pointer, size);
		pointer += size;
	}
	return *this;
}

nrg::Packet& nrg::Packet::reset(){
	memset(data, 0, used_size);
	used_size = 0;
	pointer = data;
	return *this;
}

nrg::Packet& nrg::Packet::erase(off_t offset, size_t size){
	size_t end = std::min(used_size, offset + size);
	offset = std::min<size_t>(used_size, std::max<size_t>(0, offset));
	if(end < used_size){
		memmove(data + offset, data + end, used_size - end);
	}
	return *this;
}

nrg::Packet& nrg::Packet::seek(off_t offset, int whence){
	switch(whence){
	case SEEK_CUR:
		pointer += offset; break;
	case SEEK_SET:
		pointer = data + offset; break;
	case SEEK_END:
		pointer = (data + used_size) - offset; break;
	}
	pointer = std::max(data, std::min(data + used_size, pointer));
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

