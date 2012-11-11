#ifndef NRG_ENTITY_H
#define NRG_ENTITY_H
#include "nrg_core.h"
#include "nrg_field.h"
#include <vector>

namespace nrg {

struct NRG_LIB Entity {
	virtual Entity* clone() = 0;
	virtual uint16_t getID() = 0;
	virtual void getFields(std::vector<FieldBase*>& vec) = 0;
	virtual ~Entity(){};
};

};

#endif
