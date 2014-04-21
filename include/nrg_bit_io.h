#ifndef NRG_BIT_IO_H
#define NRG_BIT_IO_H
#include "nrg_core.h"
#include "nrg_packet.h"

namespace nrg {

namespace detail {
	static const size_t MAX_BYTE_SHIFTS = 7;
}

class  BitWriter {
public:
	BitWriter(Packet& p) : bits(0), count(0), p(p){}
	void write(bool b){
		b ? write1() : write0();
	}
	void write1(){
		bits |= 1 << (detail::MAX_BYTE_SHIFTS - count);
		write0();
	}
	void write0(){
		if(++count > detail::MAX_BYTE_SHIFTS) flush();
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

class  BitReader {
public:
	BitReader(Packet& p) : bits(0), count(0), p(p){}
	bool read(void){
		if(count == 0) p.read8(bits);
		bool b = bits & (1 << (detail::MAX_BYTE_SHIFTS - count));
		count = (count + 1) & detail::MAX_BYTE_SHIFTS;
		return b;
	}
	template<class T>
	void readFunc(int sz, const T& fn){
		for(int i = 0; i < sz; ++i){
			fn(i, read());
		}
	}
private:
	uint8_t bits, count;
	Packet& p;
};

}

#endif
