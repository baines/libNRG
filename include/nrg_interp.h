#ifndef NRG_INTERP_H
#define NRG_INTERP_H
#include <type_traits>

namespace nrg {

template<class T>
struct lerp {
	T operator()(float one, float two, const double& x) const {
		return one + (two - one) * x;
	}
};

}

#endif
