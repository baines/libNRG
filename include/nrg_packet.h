#ifndef NRG_PACKET_H
#define NRG_PACKET_H
#include "nrg_core.h"
#include "nrg_netaddress.h"

namespace nrg {

struct NRG_LIB Packet {
	Packet();
	virtual ~Packet(){};
	uint8_t* data;
	size_t data_size;
};

};

#endif
