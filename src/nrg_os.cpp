#ifdef __linux
#include <time.h>
#include "nrg_os.h"

uint64_t nrg::os::microseconds(){
	struct timespec t = {};
	clock_gettime(CLOCK_MONOTONIC, &t);

	return t.tv_sec * 1000000ULL + (t.tv_nsec / 1000);
}

#endif
//TODO Windows
