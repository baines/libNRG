#include "nrg_packet.h"
#include "nrg_config.h"
#include <algorithm>
#include <iostream>

using namespace nrg;
using namespace std;

Packet::Packet() : data(new uint8_t[NRG_MAX_PACKET_SIZE]), pointer(data), 
data_size(NRG_MAX_PACKET_SIZE), used_size(0) {

}

Packet::Packet(size_t initial_size) 
: data(new uint8_t[initial_size]), pointer(data), data_size(initial_size)
, used_size(0) {

}

Packet::Packet(const Packet& copy) : data(new uint8_t[copy.data_size]), 
pointer(data + (copy.pointer - copy.data)), data_size(copy.data_size), 
used_size(copy.used_size){
	memcpy(data, copy.data, copy.used_size);
}

Packet& Packet::operator=(const Packet& other){
	reset();
	while(data_size < other.used_size) resize();
	used_size = other.used_size;
	memcpy(data, other.data, other.used_size);
	pointer = data + (other.pointer - other.data);
	return *this;
}

Packet::~Packet(){
	if(data) delete [] data;
}

PacketWritable& Packet::write8(const uint8_t& v){
	while((size_t)(pointer - data) > (data_size - sizeof(v))){
		resize();
	}
	*pointer++ = v;
	used_size = std::max(used_size, (size_t)(pointer - data));
	return *this;
}

PacketWritable& Packet::write16(const uint16_t& v){
	writeBE(hton(v));
	return *this;
}

PacketWritable& Packet::write32(const uint32_t& v){
	writeBE(hton(v));
	return *this;
}

PacketWritable& Packet::write64(const uint64_t& v){
	writeBE(hton(v));
	return *this;
}

PacketWritable& Packet::writeArray(const uint8_t* v, size_t size){
	while(data_size - (pointer - data) < size){
		resize();
	}
	memcpy(pointer, v, size);
	pointer += size;
	used_size = std::max(used_size, (size_t)(pointer - data));
	return *this;
}

PacketReadable& Packet::read8(uint8_t& v){
	if((size_t)(pointer - data) <= (used_size - sizeof(v))){
		v = *pointer++;
	}
	return *this;
}

PacketReadable& Packet::read16(uint16_t& v){
	uint16_t be_v;
	readBE(be_v);
	v = ntoh(be_v);
	return *this;
}

PacketReadable& Packet::read32(uint32_t& v){
	uint32_t be_v;
	readBE(be_v);
	v = ntoh(be_v);
	return *this;
}

PacketReadable& Packet::read64(uint64_t& v){
	uint64_t be_v;
	readBE(be_v);
	v = ntoh(be_v);
	return *this;
}

PacketReadable& Packet::readArray(uint8_t* v, size_t size){
	if((size_t)(pointer - data) <= (used_size - size)){
		memcpy(v, pointer, size);
		pointer += size;
	}
	return *this;
}

Packet& Packet::reset(){
	//memset(data, 0, used_size);
	used_size = 0;
	pointer = data;
	return *this;
}

void Packet::seek(off_t offset, int whence){
	switch(whence){
	case SEEK_CUR:
		pointer += offset; break;
	case SEEK_SET:
		pointer = data + offset; break;
	case SEEK_END:
		pointer = (data + used_size) - offset; break;
	}
	pointer = std::max(data, std::min(data + used_size, pointer));
}

off_t Packet::tell() const {
	return pointer - data;
}

void Packet::resize(){
	uint8_t* new_data = new uint8_t[data_size * 2];
	memcpy(new_data, data, data_size);
	data_size *= 2;
	pointer = new_data + (pointer - data);
	delete [] data;
	data = new_data;
}

static const int NRG_COMPRESSION_LEVEL = 3;

#ifdef NRG_ENABLE_ZLIB_COMPRESSION
#include <zlib.h>

bool PacketCompressor::apply(Packet& in, Packet& out){
	uLongf buff_size = ::compressBound(in.remaining()), len = buff_size;
	uint8_t* buff = new uint8_t[buff_size];
	::compress2(buff, &len, in.getPointer(), in.remaining(), NRG_COMPRESSION_LEVEL);
	if(len < in.size()){
		out.write8(1);
		out.write16(in.remaining());
		out.writeArray(buff, len);
	} else {
		out.write8(0);
		out.writeArray(in.getPointer(), in.remaining());
	}
	delete [] buff;
	return true;
}

bool PacketCompressor::remove(Packet& in, Packet& out){
	bool ret = false;	
	uint8_t v = 2;
	in.read8(v);
	if(v == 0){
		out.writeArray(in.getPointer(), in.remaining());
		ret = true;
	} else if(v == 1){
		uint16_t unc_len = 0;
		in.read16(unc_len);
		if(unc_len > 0){
			uint8_t* buff = new uint8_t[unc_len];
			uLongf sz = unc_len;
			if(::uncompress(buff, &sz, in.getPointer(), in.remaining()) == Z_OK){
				out.writeArray(buff, sz);
				ret = true;
			}
			delete [] buff;
		}
	}
	return ret;
}

#endif
