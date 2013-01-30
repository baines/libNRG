#include "nrg_field.h"
#include "nrg_entity.h"

using namespace nrg;

FieldBase::FieldBase(FieldContainer* c) : container(c), updated(true) {
	
}

bool FieldBase::wasUpdated() const{
	return updated;
}

void FieldBase::setUpdated(bool val){
	updated = val;
	if(updated){
		container->markUpdated();
	}
}
