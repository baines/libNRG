/*
  LibNRG - Networking for Real-time Games

  Copyright (C) 2012-2014 Alex Baines <alex@abaines.me.uk>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
/** @file
 * BitReader and BitWriter classes for easier manipulation of bitstreams
 */
#ifndef NRG_BIT_IO_H
#define NRG_BIT_IO_H
#include "nrg_core.h"
#include "nrg_packet.h"

namespace nrg {

namespace detail {
	static const size_t MAX_BYTE_SHIFTS = 7;
}

/** Writes a stream of bytes to a packet more easily */
class  BitWriter {
public:
	/** Constructor a BitWriter that will read from the Packet \p p */
	BitWriter(Packet& p) : bits(0), count(0), p(p){}

	/** Write a 1 or 0 if \p b is true or false respectively */
	void write(bool b){
		b ? write1() : write0();
	}

	/** Write a 1 bit */
	void write1(){
		bits |= 1 << (detail::MAX_BYTE_SHIFTS - count);
		write0();
	}

	/** Write a 0 bit */
	void write0(){
		if(++count > detail::MAX_BYTE_SHIFTS) flush();
	}

	/** Calls BitWriter::write using the bool return value from the function \p fn \p sz times */
	template<class T>
	void writeFunc(int sz, const T& fn){
		for(int i = 0; i < sz; ++i){
			write(fn(i));
		}
	}

	/** Finishes writing bits to the packet, and resets the internal state */
	void flush(void){
		if(!count) return;
		p.write8(bits);
		clear();
	}

	/** Resets the internal set of bits that have not yet been written */
	void clear(){
		bits = count = 0;
	}

	/** Destructor, which will call BitWriter::flush */
	~BitWriter(){
		flush();
	}
private:
	uint8_t bits, count;
	Packet& p;
};

/** Reads a stream of bytes from a packet more easily */
class  BitReader {
public:
	/** Constructs a BitReader that will read from the Packet \p p */
	BitReader(Packet& p) : bits(0), count(0), p(p){}

	/** Reads a bit from the internal state and returns a bool representation of it */
	bool read(void){
		if(count == 0) p.read8(bits);
		bool b = bits & (1 << (detail::MAX_BYTE_SHIFTS - count));
		count = (count + 1) & detail::MAX_BYTE_SHIFTS;
		return b;
	}

	/** Calls BitReader::read \p sz times, calling \p fn with each result */
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
