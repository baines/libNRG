#ifndef NRG_STATE_H
#define NRG_STATE_H
#include "nrg_core.h"

struct NRG_LIB State {
	State(ConnectionOutgoing& out);
	virtual bool addIncomingPacket(Packet& p) = 0;
	virtual bool needsUpdate() const = 0;
	virtual enum StateUpdateResult update() = 0;
	virtual ~State();
};

#endif
