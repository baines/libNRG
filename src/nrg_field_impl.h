#ifndef NRG_FIELD_IMPL_H
#define NRG_FIELD_IMPL_H
#include "nrg_field.h"

namespace nrg {

struct FieldListImpl : FieldList {
	virtual FieldList& add(FieldBase& f){
		vec.push_back(&f);
		return *this;
	} 
	std::vector<FieldBase*> vec;
};

}

#endif
