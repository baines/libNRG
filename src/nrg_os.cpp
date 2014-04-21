#ifdef __linux
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <random>
#include "nrg_os.h"

using namespace nrg;

uint64_t os::microseconds(){
	struct timespec t = {};
	clock_gettime(CLOCK_MONOTONIC, &t);

	return t.tv_sec * 1000000ULL + (t.tv_nsec / 1000);
}

uint32_t os::random(){
	uint32_t i = 0;

	int f = open("/dev/urandom", O_RDONLY);
	
	if(f < 0 || read(f, &i, sizeof(i)) < 0){
		uint64_t t = os::microseconds();
		uint32_t h = t >> 32, l = static_cast<uint32_t>(t);
		
		std::seed_seq({ l, h, static_cast<uint32_t>(rand_r(&l))})
		.generate(&i, (&i)+1);
	}
	
	if(f >= 0) close(f);
	
	return i;
}

#endif
//TODO Windows
