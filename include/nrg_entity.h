#ifndef NRG_ENTITY_H
#define NRG_ENTITY_H
#include "nrg_core.h"
#include "nrg_field.h"
#include <vector>

namespace nrg {

class EntityManager;
class Client;

class NRG_LIB Entity : public FieldContainer {
public:
	Entity();
	virtual Entity* clone() = 0;
	virtual uint16_t getType() const = 0;
	
	virtual void onCreate(Client& c){}
	virtual void onUpdate(Client& c){}
	virtual void onDelete(Client& c){}
	
	void markUpdated(bool updated);
	float getInterpTimer() const;
	
	uint16_t getID() const { return nrg_id; }
	void setID(int id){ nrg_id = id; }
	
	EntityManager* getManager() const { return manager; }
	void setManager(EntityManager* m){ manager = m; }
	virtual ~Entity();
private:
	int nrg_id;
	bool nrg_updated;
	EntityManager* manager;
};

class InputBase;

struct EntityManager {
	virtual void markEntityUpdated(Entity& e){}
	virtual void unregisterEntity(Entity& e){}
	virtual float getInterpTimer() const { return 1.0f; }
	virtual bool supportsPrediction() const { return false; }
	virtual InputBase* getInput() const { return nullptr; }
};

template<class T, uint16_t type>
struct EntityHelper : nrg::Entity {
	virtual nrg::Entity* clone(){ return new T(*static_cast<T* const>(this)); }
	virtual uint16_t getType() const { return type; }
};

}

#endif
