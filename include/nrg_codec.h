#ifndef NRG_CODEC_H
#define NRG_CODEC_H
#include "nrg_core.h"
#include <string>

namespace nrg {

template<class T>
struct Codec {
	void encode(Packet& p, const T& data){
		p.write(data);
	}
	bool decode(Packet& p, const T& data){
		if(p.remaining() >= sizeof(T)){
			p.read(data);
			return true;
		} else {
			return false;
		}
	}
};

template<size_t len>
struct Codec<char[len]> {
	void encode(Packet& p, const char (&data)[len]){
		p.writeArray(data, len);
	}
	bool decode(Packet& p, char (&data)[len]){
		if(p.remaining() >= len){
			p.readArray(&data, len);
			return true;
		} else {
			return false;
		}
	}
};

template<>
struct Codec<std::string> {
	void encode(Packet& p, const std::string& str){
		p.write<uint32_t>(str.length());
		p.writeArray(reinterpret_cast<const uint8_t*>(str.c_str()), str.length());
	}
	bool decode(Packet& p, std::string& str){
		uint32_t len = 0;
		if(p.remaining() >= sizeof(uint32_t)){
			p.read<uint32_t>(len);
			if(p.remaining() < len) return false;
			str = std::string(reinterpret_cast<const char*>(p.getPointer()), len);
			p.seek(len, SEEK_CUR);
			return true;
		} else {
			return false;
		}
	}
};

}

#endif
