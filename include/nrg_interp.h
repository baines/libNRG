#ifndef NRG_INTERP_H
#define NRG_INTERP_H
#include <iostream>

namespace nrg {

template<class T>
struct lerp {
	T operator()(const T& one, const T& two, const double& x) const {
		return one + (two - one) * x;
	}
};

}

#endif
