#ifndef NRG_CODEC_H
#define NRG_CODEC_H
#include "nrg_core.h"

namespace nrg {

template<class T>
struct Codec {
	bool encode(Packet& p, const T& data){
		p.write(data);
		return true;
	}
	bool decode(Packet& p, const T& data){
		if(p.remaining() > sizeof(T)){
			p.read(data);
			return true;
		} else {
			return false;
		}
	}
};

}

#endif
