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
/* @file
 * Contains functionality related to the Packet class, which stores data sent / received over the network
 */
#ifndef NRG_PACKET_H
#define NRG_PACKET_H
#include "nrg_core.h"
#include "nrg_config.h"
#include "nrg_netaddress.h"
#include "nrg_endian.h"
#include <cstring> // memcpy
#include <cstdio> // SEEK_x
#include <algorithm>

namespace nrg {

/** Interface for Packet's reading functions */
struct PacketReadable {
	virtual PacketReadable& read8(uint8_t& v) = 0;   /**< Read a uint8_t, automatically converting endianness */
	virtual PacketReadable& read16(uint16_t& v) = 0; /**< Read a uint16_t, automatically converting endianness */
	virtual PacketReadable& read32(uint32_t& v) = 0; /**< Read a uint32_t, automatically converting endianness */
	virtual PacketReadable& read64(uint64_t& v) = 0; /**< Read a uint64_t, automatically converting endianness */
	virtual PacketReadable& readArray(uint8_t* v, size_t size) = 0; /**< Read an array of size \p size into \p v, make sure it's big enough! */
	virtual ~PacketReadable(){} /**< Standard Destructor */
};

/** Interface for Packet's writing functions */
struct PacketWritable {
	virtual PacketWritable& write8(const uint8_t& v) = 0;   /**< Write a uint8_t, automatically converting endianness */
	virtual PacketWritable& write16(const uint16_t& v) = 0; /**< Write a uint16_t, automatically converting endianness */
	virtual PacketWritable& write32(const uint32_t& v) = 0; /**< Write a uint32_t, automatically converting endianness */
	virtual PacketWritable& write64(const uint64_t& v) = 0; /**< Write a uint64_t, automatically converting endianness */
	virtual PacketWritable& writeArray(const void* v, size_t size) = 0; /**< Write an array of size \p size, no endian conversion is performed */
	virtual ~PacketWritable(){} /**< Standard Destructor */
};

/** Class for storing data to be sent / received across the network */
class Packet : public PacketReadable, public PacketWritable {
public:
	/** Default Constructor */
	Packet();

	/** Constructor with explicit initial size */
	Packet(size_t initial_size);

	/** Copy Constructor */
	Packet(const Packet& copy);

	/** Assignment operator */
	Packet& operator=(const Packet& other);

	/** Standard Destructor */
	virtual ~Packet();

	PacketWritable& write8(const uint8_t& v);
	PacketWritable& write16(const uint16_t& v);
	PacketWritable& write32(const uint32_t& v);
	PacketWritable& write64(const uint64_t& v);
	PacketWritable& writeArray(const void* v, size_t size);

	/** Generic write function without endian conversion, be careful with types like size_t that differ across platforms! */
	template<typename T>
	void writeBE(const T& be_v){
		while((size_t)(pointer - data) > (data_size - sizeof(be_v))){
			resize();
		}
		memcpy(pointer, &be_v, sizeof(be_v));
		pointer += sizeof(be_v);
		used_size = std::max(used_size, (size_t)(pointer - data));
	}

	/** Generic write function with endian conversion, be careful with types like size_t that differ across platforms! */
	template<typename T>
	void write(const T& v){
		writeBE(nrg::hton(v));
	}

	PacketReadable& read8(uint8_t& v);
	PacketReadable& read16(uint16_t& v);
	PacketReadable& read32(uint32_t& v);
	PacketReadable& read64(uint64_t& v);
	PacketReadable& readArray(uint8_t* v, size_t size);

	/** Generic read function without endian conversion, be careful with types like size_t that differ across platforms! */
	template<typename T>
	void readBE(T& be_v){
		if((size_t)(pointer - data) <= (used_size - sizeof(be_v))){
			memcpy(&be_v, pointer, sizeof(be_v));
			pointer += sizeof(be_v);
		}
	}

	/** Generic read function with endian conversion, be careful with types like size_t that differ across platforms! */
	template<typename T>
	void read(T& v){
		T be = 0;
		readBE(be);
		v = nrg::ntoh(be);
	}

	/** Clears all data in the Packet */
	virtual Packet& reset();

	/** Seeks the packet to some offset using SEEK_SET, SEEK_CUR or SEEK_END */
	Packet& seek(off_t offset, int whence);

	/** Get the current offset of this Packet's internal pointer */
	off_t tell() const;

	/** Get the total used-size of this packet */
	size_t size() const { return used_size; }

	/** Get the amount of data that can be read from the internal pointer's current position */
	size_t remaining() const { return used_size - (pointer - data); }

	/** Returns the internal pointer */
	const uint8_t* getPointer() const { return pointer; }

	/** Returns the base address of the packet's data without affecting the internal pointer */
	const uint8_t* getBasePointer() const { return data; }
private:
	void resize();
	uint8_t *data, *pointer;
	size_t data_size, used_size;
};

/** Interface representing a transformation of a Packet, such as Compression */
struct PacketTransformation {
	virtual bool apply(Packet& in, Packet& out) = 0;
	virtual bool remove(Packet& in, Packet& out) = 0;
};

#ifdef NRG_ENABLE_ZLIB_COMPRESSION

/** Implementation of a compressing PacketTransformation using zlib */
struct PacketCompressor : PacketTransformation {
	virtual bool apply(Packet& in, Packet& out);
	virtual bool remove(Packet& in, Packet& out);
};

#endif

}

#endif
