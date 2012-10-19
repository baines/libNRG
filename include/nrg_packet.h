#ifndef NRG_PACKET_H
#define NRG_PACKET_H
#include "nrg_core.h"
#include "nrg_netaddress.h"

namespace nrg {

class NRG_LIB Packet {
public:
	Packet();
	virtual const NetAddress* getAddress();
	virtual ~Packet(){};
protected:
	uint8_t* data;
	size_t data_size;
};

};

#endif
