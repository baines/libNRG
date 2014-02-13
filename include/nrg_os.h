#ifndef NRG_OS_H
#define NRG_OS_H
#include "nrg_core.h"

namespace nrg {
namespace os {
	uint64_t microseconds();
	
	inline uint64_t milliseconds(){
		return microseconds() / 1000;
	}

	inline uint64_t seconds(){
		return milliseconds() / 1000;
	}
	
	uint32_t random();
}
}

#endif
