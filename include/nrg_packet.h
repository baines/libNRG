#ifndef NRG_PACKET_H
#define NRG_PACKET_H
#include "nrg_core.h"
#include "nrg_netaddress.h"

namespace nrg {

class NRG_LIB Packet {
public:
	Packet();
	NetAddress address;
protected:
	uint8_t* data;
};

};

#endif
