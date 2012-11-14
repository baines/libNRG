#ifndef NRG_ENTITY_H
#define NRG_ENTITY_H
#include "nrg_core.h"
#include "nrg_field.h"
#include <vector>

namespace nrg {

class NRG_LIB ServerMasterGameState;

class NRG_LIB Entity {
public:
	Entity();
	virtual Entity* clone() = 0;
	virtual uint16_t getType() = 0;
	virtual void getFields(std::vector<FieldBase*>& vec) = 0;
	virtual ~Entity(){};
	void markUpdated();

	friend class ServerMasterGameState;
	friend class ClientGameState;
private:
	int nrg_id;
	bool nrg_updated;
	ServerMasterGameState* nrg_serv_ptr;
};

};

#endif
