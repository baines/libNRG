#ifndef NRG_PACKET_H
#define NRG_PACKET_H
#include "nrg_core.h"
#include "nrg_netaddress.h"

namespace nrg {

class NRG_LIB PacketIn {
public:
	PacketIn();
	NetAddress from;
protected:
	uint8_t* data;
};

class NRG_LIB PacketOut {
public:
	PacketOut();
	NetAddress to;
protected:
	uint8_t* data;
};

};

#endif
