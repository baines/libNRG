/*
  LibNRG - Networking for Real-time Games
  
  Copyright (C) 2012-2014 Alex Baines <alex@abaines.me.uk>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#ifndef NRG_ENTITY_H
#define NRG_ENTITY_H
#include "nrg_core.h"
#include "nrg_field.h"
#include <vector>

namespace nrg {

class EntityManager;
class Client;

class Entity : public FieldContainer {
public:
	 Entity();
	 virtual Entity* clone() = 0;
	 virtual uint16_t getType() const = 0;
	
	 virtual void onCreate(Client& c){}
 	 virtual void onDestroy(Client& c){}
	 virtual void onUpdate(Client& c){}
	
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
