#ifndef NRG_PACKET_H
#define NRG_PACKET_H
#include "nrg_core.h"
#include "nrg_netaddress.h"
#include <cstring> // memcpy
#include <cstdio> // SEEK_x
#include <algorithm>

namespace nrg {

class NRG_LIB Packet {
public:
	Packet(size_t initial_size);
	virtual ~Packet();

	Packet& write8(const uint8_t& v);
	Packet& write16(const uint16_t& v);
	Packet& write32(const uint32_t& v);
	Packet& write64(const uint64_t& v);
	Packet& writeArray(const uint8_t* v, size_t size);

	template<typename T>
	Packet& writeBE(T be_v){
		while((pointer - data) > (data_size - sizeof(be_v))){
			resize();
		}
		memcpy(pointer, &be_v, sizeof(be_v));
		pointer += sizeof(be_v);
		used_size = std::max(used_size, (size_t)(pointer - data));
		return *this;
	}

	Packet& read8(uint8_t& v);
	Packet& read16(uint16_t& v);
	Packet& read32(uint32_t& v);
	Packet& read64(uint64_t& v);

	template<typename T>
	Packet& readBE(T& be_v){
		if((pointer - data) <= (used_size - sizeof(be_v))){
			memcpy(&be_v, pointer, sizeof(be_v));
			pointer += sizeof(be_v);
		}
		return *this;
	}

	virtual Packet& reset();
	Packet& seek(off_t offset, int whence);
	off_t tell() const;
	size_t size() const { return used_size; }
	size_t remaining() const { return used_size - (pointer - data); }
	const uint8_t* getPointer() const { return pointer; }
	friend class Socket;
protected:
	void resize();
	uint8_t *data, *pointer;
	size_t data_size, used_size;
};

class NRG_LIB PartialPacket : public Packet {
public:
	PartialPacket(size_t initial_size) : Packet(initial_size), complete(false){};
	virtual ~PartialPacket(){};
	void markComplete() { complete = true; }
	bool isComplete() const { return complete; }
	Packet& reset(){ Packet::reset(); complete = false; }
private:
	bool complete;
};

};

#endif
