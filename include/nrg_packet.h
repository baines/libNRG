#ifndef NRG_PACKET_H
#define NRG_PACKET_H
#include "nrg_core.h"

namespace nrg {

class PacketIn {
public:
	PacketIn();
	NetAddress from;
protected:
	uint8_t* data;
};

class PacketOut {
public:
	PacketOut();
protected:
	uint8_t* data;
};

};

#endif
