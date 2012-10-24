#ifndef NRG_PACKET_H
#define NRG_PACKET_H
#include "nrg_core.h"
#include "nrg_netaddress.h"

namespace nrg {

struct NRG_LIB Packet {
	Packet();
	Packet(uint8_t* data, size_t size);
	explicit Packet(size_t size);
	virtual ~Packet(){};
	virtual bool isValid();

	uint8_t* data;
	size_t data_size;
protected:
	bool free_data;
};

struct ConnectionPacket : public Packet {
	ConnectionPacket(const Packet& p);
	uint32_t getSequenceNumber();
	void setSequenceNumber(uint32_t num);
};

};

#endif
