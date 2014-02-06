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

struct NRG_LIB PacketReadable {
	virtual PacketReadable& read8(uint8_t& v) = 0;
	virtual PacketReadable& read16(uint16_t& v) = 0;
	virtual PacketReadable& read32(uint32_t& v) = 0;
	virtual PacketReadable& read64(uint64_t& v) = 0;
	virtual PacketReadable& readArray(uint8_t* v, size_t size) = 0;
	virtual ~PacketReadable(){}
};

struct NRG_LIB PacketWritable {
	virtual PacketWritable& write8(const uint8_t& v) = 0;
	virtual PacketWritable& write16(const uint16_t& v) = 0;
	virtual PacketWritable& write32(const uint32_t& v) = 0;
	virtual PacketWritable& write64(const uint64_t& v) = 0;
	virtual PacketWritable& writeArray(const uint8_t* v, size_t size) = 0;
	virtual ~PacketWritable(){}
};

class NRG_LIB Packet : public PacketReadable, public PacketWritable {
public:
	Packet();
	Packet(size_t initial_size);
	Packet(const Packet& copy);
	Packet& operator=(const Packet& other);
	virtual ~Packet();

	PacketWritable& write8(const uint8_t& v);
	PacketWritable& write16(const uint16_t& v);
	PacketWritable& write32(const uint32_t& v);
	PacketWritable& write64(const uint64_t& v);
	PacketWritable& writeArray(const uint8_t* v, size_t size);

	template<typename T>
	void writeBE(const T& be_v){
		while((size_t)(pointer - data) > (data_size - sizeof(be_v))){
			resize();
		}
		memcpy(pointer, &be_v, sizeof(be_v));
		pointer += sizeof(be_v);
		used_size = std::max(used_size, (size_t)(pointer - data));
	}

	template<typename T>
	void write(const T& v){
		writeBE(nrg::hton(v));
	}

	PacketReadable& read8(uint8_t& v);
	PacketReadable& read16(uint16_t& v);
	PacketReadable& read32(uint32_t& v);
	PacketReadable& read64(uint64_t& v);
	PacketReadable& readArray(uint8_t* v, size_t size);

	template<typename T>
	void readBE(T& be_v){
		if((size_t)(pointer - data) <= (used_size - sizeof(be_v))){
			memcpy(&be_v, pointer, sizeof(be_v));
			pointer += sizeof(be_v);
		}
	}

	template<typename T>
	void read(T& v){
		T be = 0;
		readBE(be);
		v = nrg::ntoh(be);
	}

	virtual Packet& reset();
	void seek(off_t offset, int whence);
	off_t tell() const;
	size_t size() const { return used_size; }
	size_t remaining() const { return used_size - (pointer - data); }
	const uint8_t* getPointer() const { return pointer; }
	const uint8_t* getBasePointer() const { return data; }
protected:
	void resize();
	uint8_t *data, *pointer;
	size_t data_size, used_size;
};

struct NRG_LIB PacketTransformation {
	virtual bool apply(Packet& in, Packet& out) = 0;
	virtual bool remove(Packet& in, Packet& out) = 0;
};

#ifdef NRG_ENABLE_ZLIB_COMPRESSION

struct NRG_LIB PacketCompressor : PacketTransformation {
	virtual bool apply(Packet& in, Packet& out);
	virtual bool remove(Packet& in, Packet& out);
};

#endif

}

#endif
