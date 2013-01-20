#ifndef NRG_FIELD_IMPL_H
#define NRG_FIELD_IMPL_H
#include "nrg_field.h"

namespace nrg {

struct FieldListImpl : FieldList {
	virtual FieldList& add(FieldBase& f){
		vec.push_back(&f);
		return *this;
	}
	virtual size_t size() const {
		return vec.size();
	}
	virtual FieldBase* get(size_t index){
		return index >= size() ? NULL : vec[index];
	}
	std::vector<FieldBase*> vec;
};

}

#endif
