#ifndef NRG_FIELD_H
#define NRG_FIELD_H
#include "nrg_core.h"
#include "nrg_packet.h"
#include "nrg_codec.h"

namespace nrg {

class NRG_LIB Entity;

class NRG_LIB FieldBase {
public:
	FieldBase(Entity* containing_entity);
	virtual size_t readFromPacket(Packet& p) = 0;
	virtual size_t writeToPacket(Packet& p) const = 0;
	virtual ~FieldBase(){};

	bool wasUpdated() const;
	void setUpdated(bool updated);
private:
	Entity* containing_entity;
	bool updated;
};

struct NRG_LIB FieldList {
	virtual FieldList& add(FieldBase& f) = 0;
};

template<typename T, class Cdc = nrg::Codec<T> >
class Field : public FieldBase {
public:
	Field(Entity* e) : FieldBase(e), data(){};
	Field(Entity* e, const T& t) : FieldBase(e), data(t){};

	virtual size_t readFromPacket(Packet& p){
		return Cdc().decode(p, data);
	}

	virtual size_t writeToPacket(Packet& p) const {
		return Cdc().encode(p, data);
	}

	void set(const T& other){
		data = other;
		this->setUpdated(true);
	}

	Field& operator=(const T& other){
		data = other;
		this->setUpdated(true);
		return *this;
	}
	
	T get() const {
		return data;
	}
private:
	T data;
};

};

#endif
