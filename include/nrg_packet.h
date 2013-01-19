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

class NRG_LIB Packet {
public:
	Packet();
	Packet(size_t initial_size);
	Packet(const Packet& copy);
	Packet& operator=(const Packet& other);
	virtual ~Packet();

	Packet& write8(const uint8_t& v);
	Packet& write16(const uint16_t& v);
	Packet& write32(const uint32_t& v);
	Packet& write64(const uint64_t& v);
	Packet& writeArray(const uint8_t* v, size_t size);

	template<typename T>
	Packet& writeBE(T be_v){
		while((size_t)(pointer - data) > (data_size - sizeof(be_v))){
			resize();
		}
		memcpy(pointer, &be_v, sizeof(be_v));
		pointer += sizeof(be_v);
		used_size = std::max(used_size, (size_t)(pointer - data));
		return *this;
	}

	template<typename T>
	Packet& write(const T& v){
		return writeBE(nrg::hton(v));
	}

	Packet& read8(uint8_t& v);
	Packet& read16(uint16_t& v);
	Packet& read32(uint32_t& v);
	Packet& read64(uint64_t& v);
	Packet& readArray(uint8_t* v, size_t size);

	template<typename T>
	Packet& readBE(T& be_v){
		if((size_t)(pointer - data) <= (used_size - sizeof(be_v))){
			memcpy(&be_v, pointer, sizeof(be_v));
			pointer += sizeof(be_v);
		}
		return *this;
	}

	template<typename T>
	Packet& read(T& v){
		T be = 0;
		readBE(be);
		v = nrg::ntoh(be);
		return *this;
	}

	virtual Packet& reset();
	Packet& seek(off_t offset, int whence);
	off_t tell() const;
	size_t size() const { return used_size; }
	size_t remaining() const { return used_size - (pointer - data); }
	const uint8_t* getPointer() const { return pointer; }
	const uint8_t* getBasePointer() const { return data; }
	friend class Socket;
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

};

#endif
