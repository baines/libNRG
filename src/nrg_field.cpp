#include "nrg_field.h"
#include "nrg_entity.h"

using namespace nrg;

namespace {

template<class T, class S>
static T* OFF(const T* a, const S* b, const S* c){
	return (T*)((char*)a + ((char*)b-(char*)c));
}

}

FieldBase::FieldBase(FieldContainer* c) 
: container(c)
, next(NULL)
, updated(true) {
	container->addField(this);
}

FieldBase::FieldBase(const FieldBase& copy) 
: container(OFF(copy.container, this, &copy))
, next(copy.next ? OFF(copy.next, this, &copy) : NULL)
, updated(true){

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

FieldBase* FieldBase::getNextField() const {
	return next;
}

void FieldBase::setNextField(FieldBase* f){
	next = f;
}

FieldContainer::FieldContainer()
: field_head(NULL)
, num_fields(0) {

}

FieldContainer::FieldContainer(const FieldContainer& copy) 
: field_head(copy.field_head ? OFF(copy.field_head, this, &copy) : NULL)
, num_fields(copy.num_fields) {

}

FieldBase* FieldContainer::getFirstField() const {
	return field_head;
}

size_t FieldContainer::getNumFields() const {
	return num_fields;
}

void FieldContainer::addField(FieldBase* f){
	if(!field_head){
		field_head = f;
	} else {
		FieldBase* f2 = field_head, *f3;
		while((f3 = f2->getNextField())){
			f2 = f3;
		}
		f2->setNextField(f);
	}
	++num_fields;
}
