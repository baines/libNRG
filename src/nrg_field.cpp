#include "nrg_field.h"
#include "nrg_entity.h"

using namespace nrg;

FieldBase::FieldBase(Entity* e) : containing_entity(e), updated(true) {
	
}

bool FieldBase::wasUpdated() const{
	return updated;
}

void FieldBase::setUpdated(bool val){
	updated = val;
	if(updated){
		containing_entity->markUpdated();
	}
}
