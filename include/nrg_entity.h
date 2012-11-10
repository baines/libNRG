#ifndef NRG_ENTITY_H
#define NRG_ENTITY_H

namespace nrg {

struct NRG_LIB Entity {
	Entity(){};
	virtual Entity* clone() = 0;
	virtual uint16_t getID() = 0;
};

};

#endif
