#ifndef NRG_FIELD_H
#define NRG_FIELD_H
#include "nrg_core.h"
#include "nrg_packet.h"
#include "nrg_codec.h"
#include "nrg_util.h"
#include "nrg_interp.h"
#include <bitset>

namespace nrg {

class NRG_LIB FieldContainer;

class NRG_LIB FieldBase {
public:
	FieldBase(FieldContainer* container);
	FieldBase(const FieldBase& copy);
	FieldBase& operator=(const FieldBase& copy);
	virtual size_t readFromPacket(Packet& p) = 0;
	virtual size_t writeToPacket(Packet& p) const = 0;
	virtual void shiftData() = 0;
	virtual ~FieldBase(){}

	virtual bool wasUpdated() const;
	virtual void setUpdated(bool updated);
	FieldBase* getNextField() const;
	void setNextField(FieldBase* f);
protected:
	FieldContainer* container;
	FieldBase* next;
	bool updated;
};

struct NRG_LIB FieldContainer {
	FieldContainer();
	FieldContainer(const FieldContainer& copy);
	FieldContainer& operator=(const FieldContainer& copy);
	virtual void markUpdated(bool b) = 0;
	virtual float getInterpTimer() const { return 1.0f; }
	FieldBase* getFirstField() const;
	size_t getNumFields() const;
	void addField(FieldBase* f);
private:
	FieldBase* field_head;
	size_t num_fields;
};

template<typename T, class Cdc = nrg::Codec<T> >
class Field : public FieldBase {
public:
	Field(FieldContainer* c) : FieldBase(c), data(), data_next(){}
	Field(FieldContainer* c, const T& t) : FieldBase(c), data(t), data_next(t){}

	virtual size_t readFromPacket(Packet& p){
		return Cdc().decode(p, data_next);
	}

	virtual size_t writeToPacket(Packet& p) const {
		return Cdc().encode(p, data);
	}

	virtual void shiftData(){
		data = data_next;
	}

	void set(const T& other){
		*this = other;
	}

	Field& operator=(const T& other){
		if(data != other){
			data = data_next = other;
			this->setUpdated(true);
		}
		return *this;
	}
	
	T get() const {
		return data_next;
	}

	template<class F>
	T getInterp(const F& func) const {
		return func(data, data_next, this->container->getInterpTimer());
	}

	T getInterp() const {
		return lerp<T>()(data, data_next, this->container->getInterpTimer());
	}
private:
	T data, data_next;
};

template<typename T, size_t N>
class Field<T[N]> : public FieldBase {
public:
	Field(FieldContainer* c) : FieldBase(c), data(){}
	Field(FieldContainer* c, const T (&t)[N]) : FieldBase(c), data(t){}

	//TODO: read/write whole array if (index, val) pairs will take up more space
	virtual size_t readFromPacket(Packet& p){
		index_t count;		
		p.read<index_t>(count);
		for(size_t i = 0; i < count+1; ++i){
			index_t k;
			T v;
			p.read<index_t>(k);
			p.read<T>(v);
			data_next[k] = v;
		}
		return sizeof(index_t) + (count * (sizeof(index_t) + sizeof(T)));
	}

	virtual size_t writeToPacket(Packet& p) const {
		p.write<index_t>(updated_indices.count()-1);
		for(size_t i = 0; i < N; ++i){
			if(updated_indices[i]){
				p.write<index_t>(i);
				p.write<T>(data[i]);
			}
		}
		return sizeof(index_t) + (updated_indices.count() * (sizeof(index_t) + sizeof(T)));
	}

	virtual void shiftData(){
		memcpy(data, data_next, N*sizeof(T));
	}

	virtual void setUpdated(bool updated){
		FieldBase::setUpdated(updated);
		if(!updated) updated_indices.reset();
	}

	void set(size_t index, const T& other){
		if(data[index] != other){
			data[index] = other;
			updated_indices.set(index);
			FieldBase::setUpdated(true);
		}
	}

	Field& operator=(const T (&other)[N]){
		data = other;
		FieldBase::setUpdated(true);
		return *this;
	}
	
	T get(size_t index) const {
		return data_next[index];
	}
	
	template<class F>
	T getInterp(size_t index, const F& func) const {
		return func(data[index], data_next[index], this->container->getInterpTimer());
	}

	T getInterp(size_t index) const {
		return lerp<T>()(data[index], data_next[index], this->container->getInterpTimer());
	}
private:
	T data[N], data_next[N];
	std::bitset<N> updated_indices;
	typedef typename size2type<min_sizeof<N-1>::val>::type index_t;
};

}

#endif
