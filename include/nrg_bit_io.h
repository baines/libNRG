#ifndef NRG_BIT_IO_H
#define NRG_BIT_IO_H
#include "nrg_core.h"
#include "nrg_packet.h"

namespace nrg {

class NRG_LIB BitWriter {
	static const size_t MAX_BYTE_SHIFTS = 7;
public:
	BitWriter(nrg::Packet& p) : bits(0), count(0), p(p){};
	void write(bool b){
		b ? write1() : write0();
	}
	void write1(){
		bits |= 1 << (MAX_BYTE_SHIFTS - count);
		write0();
	}
	void write0(){
		if(++count > MAX_BYTE_SHIFTS) flush();
	}
	template<class T>
	void writeFunc(int sz, const T& fn){
		for(int i = 0; i < sz; ++i){
			write(fn(i));
		}
	}
	void flush(void){
		if(!count) return;
		p.write8(bits);
		clear();
	}
	void clear(){
		bits = count = 0;
	}
	~BitWriter(){
		flush();
	}
private:
	uint8_t bits, count;
	Packet& p;
};

}

#endif
