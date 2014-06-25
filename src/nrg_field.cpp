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
		container->markUpdated(true);
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
