#ifndef NRG_FIELD_H
#define NRG_FIELD_H
#include "nrg_core.h"
#include "nrg_packet.h"
#include "nrg_codec.h"
#include "nrg_util.h"
#include <bitset>

namespace nrg {

class NRG_LIB Entity;

class NRG_LIB FieldBase {
public:
	FieldBase(Entity* containing_entity);
	virtual size_t readFromPacket(Packet& p) = 0;
	virtual size_t writeToPacket(Packet& p) const = 0;
	virtual ~FieldBase(){};

	virtual bool wasUpdated() const;
	virtual void setUpdated(bool updated);
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
		*this = other;
	}

	Field& operator=(const T& other){
		if(data != other){
			data = other;
			this->setUpdated(true);
		}
		return *this;
	}
	
	T get() const {
		return data;
	}
private:
	T data;
};

template<typename T, size_t N>
class Field<T[N]> : public FieldBase {
public:
	Field(Entity* e) : FieldBase(e), data(){};
	Field(Entity* e, const T (&t)[N]) : FieldBase(e), data(t){};

	//TODO: read/write whole array if (index, val) pairs will take up more space
	virtual size_t readFromPacket(Packet& p){
		index_t count;		
		p.read<index_t>(count);
		for(size_t i = 0; i < count; ++i){
			index_t k;
			T v;
			p.read<index_t>(k);
			p.read<T>(v);
			data[k] = v;
		}
		return sizeof(index_t) + (count * (sizeof(index_t) + sizeof(T)));
	}

	virtual size_t writeToPacket(Packet& p) const {
		p.write<index_t>(updated_indices.count());
		for(size_t i = 0; i < N; ++i){
			if(updated_indices[i]){
				p.write<index_t>(i);
				p.write<T>(data[i]);
			}
		}
		return sizeof(index_t) + (updated_indices.count() * (sizeof(index_t) + sizeof(T)));
	}

	virtual void setUpdated(bool updated){
		FieldBase::setUpdated(updated);
		if(!updated) updated_indices.reset();
	}

	void set(size_t index, const T& other){
		data[index] = other;
		updated_indices.set(index);
		FieldBase::setUpdated(true);
	}

	Field& operator=(const T (&other)[N]){
		data = other;
		FieldBase::setUpdated(true);
		return *this;
	}
	
	T get(size_t index) const {
		return data[index];
	}
private:
	T data[N];
	std::bitset<N> updated_indices;
	typedef typename size2type<min_sizeof<N>::val>::type index_t;
};

};

#endif
