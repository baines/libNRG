#ifdef __linux
#include <time.h>
#include <cstdio>
#include "nrg_os.h"

uint64_t nrg::os::microseconds(){
	struct timespec t = {};
	clock_gettime(CLOCK_MONOTONIC, &t);

	return t.tv_sec * 1000000ULL + (t.tv_nsec / 1000);
}

uint32_t nrg::os::random(){
	uint32_t r = 0;	

	FILE* f = fopen("/dev/urandom", "rb");
	fread(&r, sizeof(r), 1, f);
	fclose(f);
	
	return r;
}

#endif
//TODO Windows
