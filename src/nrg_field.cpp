#include "nrg_field.h"
#include "nrg_entity.h"

nrg::FieldBase::FieldBase(Entity* e) : containing_entity(e), updated(true) {
	
}

bool nrg::FieldBase::wasUpdated() const{
	return updated;
}

void nrg::FieldBase::setUpdated(bool val){
	updated = val;
	if(updated){
		containing_entity->markUpdated();
	}
}
