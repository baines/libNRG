#ifndef NRG_LINUX_H
#define NRG_LINUX_H
#include "../nrg_core.h"
#include <time.h>

namespace nrg {
namespace os {

uint64_t microseconds(){
	struct timespec t = {};
	clock_gettime(CLOCK_MONOTONIC, &t);

	return t.tv_sec * 1000000ULL + (t.tv_nsec / 1000);
}

};
};


#endif
