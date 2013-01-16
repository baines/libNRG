#ifndef NRG_ENTITY_H
#define NRG_ENTITY_H
#include "nrg_core.h"
#include "nrg_field.h"
#include <vector>

namespace nrg {

class NRG_LIB Server;

class NRG_LIB Entity {
public:
	Entity();
	virtual Entity* clone() = 0;
	virtual uint16_t getType() const = 0;
	virtual void getFields(FieldList& list) = 0;
	virtual ~Entity();
	void markUpdated();
	uint16_t getID() const { return nrg_id; }

	friend class Server;
	friend class ClientGameState;
private:
	int nrg_id;
	bool nrg_updated;
	Server* nrg_serv_ptr;
};

template<class T, uint16_t type>
struct EntityHelper : nrg::Entity {
	virtual nrg::Entity* clone(){ return new T(*static_cast<T*>(this)); }
	virtual uint16_t getType() const { return type; }
};

};

#endif
