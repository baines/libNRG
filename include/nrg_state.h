#ifndef NRG_STATE_H
#define NRG_STATE_H
#include "nrg_core.h"
#include "nrg_connection.h"
#include "nrg_packet.h"

namespace nrg {

class NRG_LIB State {
public:
	State(ConnectionOutgoing& out) : out(out){};
	virtual bool addIncomingPacket(Packet& p) = 0;
	virtual bool needsUpdate() const = 0;
	virtual int update() = 0;
	virtual ~State(){};
protected:
	ConnectionOutgoing& out;
};

};

#endif
