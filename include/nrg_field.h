#ifndef NRG_FIELD_H
#define NRG_FIELD_H
#include "nrg_core.h"
#include "nrg_packet.h"

namespace nrg {

struct NRG_LIB FieldBase {
	virtual size_t getSize() = 0;
	virtual void readFromPacket(Packet& p) = 0;
	virtual void writeToPacket(Packet& p) = 0;
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

	Field& operator=(const T& other){
		data = other;
		return *this;
	}
protected:
	T data;
};

};

#endif
