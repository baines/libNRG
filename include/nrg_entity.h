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
/** @file
 *  Contains classes related to nrg's Entity data-replication abstraction
 */
#ifndef NRG_ENTITY_H
#define NRG_ENTITY_H
#include "nrg_core.h"
#include "nrg_field.h"
#include <vector>

namespace nrg {

class EntityManager;
class Client;

/** Abstract class to be inherited by users of the library which acts as a container of one or more Fields */
class Entity : public FieldContainer {
public:

	/** Default Constructor */
	Entity();
	
	/** Returns an identical copy of the derived class of this Entity */
	virtual Entity* clone() = 0;
	
	/** Returns this entity's user-defined type identifier */
	virtual uint16_t getType() const = 0;
	
	/** Returns this Entity's ID assigned by the library */
	uint16_t getID() const { return nrg_id; }
	
	/** @{ */
	/** Virtual function called on the client-side when the Entity is created */
	virtual void onCreate(Client& c){}
	/** Virtual function called on the client-side just before the Entity is destroyed */
 	virtual void onDestroy(Client& c){}
 	/** Virtual function called on the client-side when the Entity is updated */
	virtual void onUpdate(Client& c){}
	/** @} */

	/** Standard Destructor */
	virtual ~Entity();
	
	/** @cond INTERNAL_USE_ONLY */
	void markUpdated(bool updated);
	double getInterpTimer() const;
	/** Sets this Entity's ID */
	void setID(int id){ nrg_id = id; }
	/** Returns this Entity's EntityManager */
	EntityManager* getManager() const { return manager; }
	/** Sets this Entity's EntityManager */
	void setManager(EntityManager* m){ manager = m; }
	/** @endcond */
	
private:
	int nrg_id;
	bool nrg_updated;
	EntityManager* manager;
};

class InputBase;

/** Abstract class that contains functionality required by Entity objects */
struct EntityManager {
	virtual void markEntityUpdated(Entity& e){}
	virtual void unregisterEntity(Entity& e){}
	virtual double getInterpTimer() const { return 1.0; }
	virtual bool supportsPrediction() const { return false; }
	virtual InputBase* getInput() const { return nullptr; }
};

/** Helper template class that automatically implements the Entity::clone and Entity::getType methods */
template<class T, uint16_t type>
struct EntityHelper : nrg::Entity {
	virtual nrg::Entity* clone(){ return new T(*static_cast<T* const>(this)); }
	virtual uint16_t getType() const { return type; }
};

}

#endif
