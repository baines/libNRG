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
/** @file
 *  Contains classes related to the Field template class that encapsulates data to be replicated across the network
 */
#ifndef NRG_FIELD_H
#define NRG_FIELD_H
#include "nrg_core.h"
#include "nrg_packet.h"
#include "nrg_codec.h"
#include "nrg_util.h"
#include "nrg_interp.h"
#include <bitset>

namespace nrg {

class FieldContainer;

/** Abstract base class that Field inherits from */
class FieldBase {
public:
	/** Standard Constructor */
	FieldBase(FieldContainer* container);
	
	/** Copy Constructor */
	FieldBase(const FieldBase& copy);
	
	/** Assignment operator */
	FieldBase& operator=(const FieldBase& copy);
	
	/** Reads data from Packet \p p into this Field */
	virtual size_t readFromPacket(Packet& p) = 0;
	
	/** Writes this Field into the Packet \p p */
	virtual size_t writeToPacket(Packet& p) const = 0;
	
	/** Indicates a new Snapshot has begun and the Field should move data_next to data */
	virtual void shiftData() = 0;
	
	/** Standard Destructor */
	virtual ~FieldBase(){}

	/** Returns true if the Field was updated since the last update */
	virtual bool wasUpdated() const;
	
	/** Force this Field's updated flag to be true or false */
	virtual void setUpdated(bool updated);
	
	/** Returns the next Field in the FielcContainer's linked list, or nullptr at the end of the list */
	FieldBase* getNextField() const;
	
	/** @cond INTERNAL_USE_ONLY */
	void setNextField(FieldBase* f);
	/** @endcond */
protected:
	FieldContainer* container;
	FieldBase* next;
	bool updated;
};

/** Abstract class used by anything that contains Fields, like Entity or InputBase */
struct FieldContainer {
	/** Standard Constructor */
	FieldContainer();
	
	/** Copy Constructor */
	FieldContainer(const FieldContainer& copy);
	
	/** Assignment Operator */
	FieldContainer& operator=(const FieldContainer& copy);

	/** Return the first field in this FieldContainer's internal linked-list */
	FieldBase* getFirstField() const;
	
	/** Returns the number of fields in this FieldContainer's linked-list */
	size_t getNumFields() const;
	
	/** @cond INTERNAL_USE_ONLY */
	/** Marks the FieldContainer as updated so the Server knows to send modified information contained inside */
	virtual void markUpdated(bool b) = 0;
	/** Passes down the Client's current progress between it's two latest snapshots to contained Fields */
	virtual double getInterpTimer() const { return 1.0; }
	/** Add a Field to this container, performed automatically during Field initialisation */
	void addField(FieldBase* f);
	/** @endcond */
private:
	FieldBase* field_head;
	size_t num_fields;
};

/** Template class encapsulating a type that will be replicated from Server to Clients, and is stored inside a FieldContainer */
template<typename T, class Cdc = nrg::Codec<T> >
class Field : private FieldBase {
public:
	/* Standard Constructor */
	Field(FieldContainer* c) : FieldBase(c), data(), data_next(){}
	
	/* Constructor with specified initial value \p t */
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

	/** Alternative to Field::operator= */
	void set(const T& other){
		*this = other;
	}

	/** Assignment operator, which also marks this field as having been updated */
	Field& operator=(const T& other){
		if(data != other){
			data = data_next = other;
			this->setUpdated(true);
		}
		return *this;
	}
	
	/** Returns the most up-to-date data without interpolation */
	T get() const {
		return data_next;
	}

	/** Returns data interpolated between the two latest Snapshots using \p func */
	template<class F>
	T getInterp(const F& func) const {
		return func(data, data_next, this->container->getInterpTimer());
	}

	/** Returns data linearly interpolated between the two latest Snapshots using nrg::lerp */
	T getInterp() const {
		return lerp<T>()(data, data_next, this->container->getInterpTimer());
	}
private:
	T data, data_next;
};

/** Specialisation of Field for array types */
template<typename T, size_t N>
class Field<T[N]> : private FieldBase {
public:
	Field(FieldContainer* c) : FieldBase(c), data(){
		updated_indices.set();
	}
	Field(FieldContainer* c, const T (&t)[N]) : FieldBase(c), data(t){
		updated_indices.set();
	}

	virtual size_t readFromPacket(Packet& p){
		index_t count;
		p.read<index_t>(count);
		size_t uic = count + 1;
		
		if(uic * (sizeof(index_t) + sizeof(T)) > N * sizeof(T)){
			for(size_t i = 0; i < N; ++i){
				p.read<T>(data_next[i]);
			}
			return sizeof(index_t) + N * sizeof(T);
		} else {
			for(size_t i = 0; i < uic; ++i){
				index_t k;
				p.read<index_t>(k);
				if(k < N){
					p.read<T>(data_next[k]);
				} else { // out of bounds for some reason, don't crash.
					p.seek(SEEK_CUR, sizeof(T));
				}
			}
			return sizeof(index_t) + (uic * (sizeof(index_t) + sizeof(T)));
		}
	}

	virtual size_t writeToPacket(Packet& p) const {
		size_t uic = updated_indices.count();
		p.write<index_t>(uic-1);
		
		if(uic * (sizeof(index_t) + sizeof(T)) > N * sizeof(T)){
			for(size_t i = 0; i < N; ++i){
				p.write<T>(data[i]);
			}
			return sizeof(index_t) + (N * sizeof(T));
		} else {
			for(size_t i = 0; i < N; ++i){
				if(updated_indices[i]){
					p.write<index_t>(i);
					p.write<T>(data[i]);
				}
			}
			return sizeof(index_t) + (uic * (sizeof(index_t) + sizeof(T)));
		}
	}

	virtual void shiftData(){
		memcpy(data, data_next, N * sizeof(T));
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
