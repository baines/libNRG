#ifndef NRG_FIELD_H
#define NRG_FIELD_H
#include "nrg_core.h"
#include "nrg_packet.h"

namespace nrg {

struct NRG_LIB FieldBase {
	virtual size_t getSize() = 0;
	virtual void readFromPacket(Packet& p) = 0;
	virtual void writeToPacket(Packet& p) = 0;
	virtual bool wasUpdated() = 0;
	virtual void setUpdated(bool updated) = 0;
	virtual ~FieldBase(){};
};

template<typename T>
class Field {
public:
	Field() : data(){};
	Field(const T& t) : data(t){};
	
	virtual size_t getSize(){
		return sizeof(T);
	}

	virtual void readFromPacket(Packet& p){

	}

	virtual void writeToPacket(Packet& p){

	}

	virtual bool wasUpdated(){
		return updated;
	}

	virtual void setUpdated(bool val){
		updated = val;
	}

	void set(const T& other){
		data = other;
		updated = true;
	}

	Field& operator=(const T& other){
		data = other;
		updated = true;
		return *this;
	}
	
	T get(){
		return data;
	}
protected:
	T data;
	bool updated;
};

};

#endif
