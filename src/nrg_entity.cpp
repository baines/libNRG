#include "nrg_entity.h"
#include "nrg_server.h"
#include "nrg_client_state.h"

using namespace nrg;

Entity::Entity()
: nrg_id(0)
, nrg_updated(false)
, manager(nullptr) {

}

void Entity::markUpdated(bool b){
	if(!nrg_updated && b && manager){
		manager->markEntityUpdated(*this);
	}
	nrg_updated = b;
}

Entity::~Entity(){
	if(manager)	manager->unregisterEntity(*this);
}

float Entity::getInterpTimer() const {
	return manager ? manager->getInterpTimer() : 1.0;
}


