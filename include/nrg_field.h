#ifndef NRG_FIELD_H
#define NRG_FIELD_H
#include "nrg_core.h"
#include "nrg_packet.h"

namespace nrg {

class NRG_LIB Entity;

class NRG_LIB FieldBase {
public:
	FieldBase(Entity* containing_entity);
	virtual size_t getSize() = 0;
	virtual void readFromPacket(Packet& p) = 0;
	virtual void writeToPacket(Packet& p) = 0;
	virtual ~FieldBase(){};

	bool wasUpdated();
	void setUpdated(bool updated);
private:
	Entity* containing_entity;
	bool updated;
};

template<typename T>
class Field {
public:
	Field(Entity* e) : FieldBase(e), data(){};
	Field(Entity* e, const T& t) : FieldBase(e), data(t){};
	
	virtual size_t getSize(){
		return sizeof(T);
	}

	virtual void readFromPacket(Packet& p){

	}

	virtual void writeToPacket(Packet& p){

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
	
	T get(){
		return data;
	}
protected:
	T data;
};

};

#endif