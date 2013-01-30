#ifndef NRG_PLAYER_H
#define NRG_PLAYER_H

namespace nrg {

struct NRG_LIB Player {
	virtual uint16_t getID() const = 0;
	virtual void kick(const char* reason) = 0;
	virtual int getPing() const = 0;
};

}

#endif
