#ifndef NRG_UTIL_H
#define NRG_UTIL_H

namespace nrg {

template<class T, uint16_t type>
struct EntityHelper : nrg::Entity {
	virtual nrg::Entity* clone(){ return new T(*static_cast<T*>(this)); }
	virtual uint16_t getType() const { return type; }
};

}

#endif

