#ifndef NRG_PACKET_H
#define NRG_PACKET_H
#include "nrg_core.h"
#include "nrg_netaddress.h"

namespace nrg {

class NRG_LIB Packet {
public:
	Packet(size_t initial_size);

	Packet& write8(uint8_t v);
	Packet& write16(uint16_t v);
	Packet& write32(uint32_t v);
	Packet& write64(uint64_t v);
	Packet& writeArray(uint8_t* v, size_t size);

	Packet& read8(uint8_t& v);
	Packet& read16(uint16_t& v);
	Packet& read32(uint32_t& v);
	Packet& read64(uint64_t& v);

	Packet& seek(off_t offset, int whence);
	off_t tell();
protected:
	uint8_t *data, *pointer;
	size_t data_size;
};

};

#endif
