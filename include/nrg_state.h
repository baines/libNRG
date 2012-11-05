#ifndef NRG_STATE_H
#define NRG_STATE_H
#include "nrg_core.h"
#include "nrg_connection.h"
#include "nrg_packet.h"

namespace nrg {

class NRG_LIB State {
public:
	State(){};
	
	typedef enum {
		STATE_EXIT_FAILURE = -1,
		STATE_EXIT_SUCCESS = 0,
		STATE_CONTINUE = 1
	} UpdateResult;
	
	virtual bool addIncomingPacket(Packet& p) = 0;
	virtual bool needsUpdate() const = 0;
	virtual size_t getTimeoutSeconds() const { return 10; }
	virtual UpdateResult update(ConnectionOutgoing& out) = 0;
	virtual ~State(){};
};

class NRG_LIB ClientHandshakeState : public State {
public:
	ClientHandshakeState() : phase(NOT_STARTED){};
	bool addIncomingPacket(Packet& p){
		uint8_t v = 0;
		if(phase != WAITING_ON_RESPONSE || p.size() < 1) return false;
		p.read8(v);
		if(v != 0) phase = static_cast<HandShakePhase>(v);
	}
	
	bool needsUpdate() const {
		return (phase != WAITING_ON_RESPONSE);
	}
	
	UpdateResult update(ConnectionOutgoing& out){
		UpdateResult res;
		if(phase == NOT_STARTED){
			Packet p(1);
			p.write8(1);
			out.sendPacket(p);
			phase = WAITING_ON_RESPONSE;
			res = STATE_CONTINUE;
		} else if(phase == ACCEPTED){
			res = STATE_EXIT_SUCCESS;
		} else {
			res = STATE_EXIT_FAILURE;
		}
		return res;
	}
private:
	enum HandShakePhase {
		NOT_STARTED = -1,
		WAITING_ON_RESPONSE = 0,
		ACCEPTED = 1,
		REJECTED_FULL = 2
	} phase;
};

};

#endif
