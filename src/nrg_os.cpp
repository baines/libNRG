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
	if(f > 0 && read(f, &i, sizeof(i)) > 0){
		close(f);
		return i;
	} else {
		return std::random_device()();
	}
}

#endif
//TODO Windows
