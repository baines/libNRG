#ifndef NRG_INTERP_H
#define NRG_INTERP_H

namespace nrg {

template<class T>
struct lerp {
	T operator()(const T& one, const T& two, const double& x) const {
		return (1.0 - x) * one + x * two;
	}
};

}

#endif
