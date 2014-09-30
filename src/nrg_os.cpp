/*
  LibNRG - Networking for Real-time Games

  Copyright (C) 2012-2014 Alex Baines <alex@abaines.me.uk>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#include "nrg_os.h"
#include <random>
using namespace nrg;

#if defined(__linux)
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

Status os::init(void){
	return StatusOK();
}

void os::uninit(void){

}

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

#elif defined(_WIN32)

static bool inited = false;
static uint64_t perf_freq = 1;

Status os::init(void){
	if(!inited){
		LARGE_INTEGER f;
		QueryPerformanceFrequency(&f);
		perf_freq = (double)f.QuadPart / 1000000;

		WSADATA stuff;

		if(WSAStartup(MAKEWORD(2,2), &stuff) != 0){
			return Status("WSAStartup Failed");
		} else {
			inited = true;
			return StatusOK();
		}
	} else {
		return StatusOK();
	}
}

void os::uninit(void){
	if(inited){
		WSACleanup();
		inited = false;
	}
}

uint64_t os::microseconds(){
	LARGE_INTEGER perf_count;
	QueryPerformanceCounter(&perf_count);

	return perf_count.QuadPart / perf_freq;
}

uint32_t os::random(){
	uint32_t i = 0;

	uint64_t t = os::microseconds();
	uint32_t h = t >> 32, l = static_cast<uint32_t>(t);

	std::seed_seq({ l, h, static_cast<uint32_t>(rand_r(&l))})
	.generate(&i, (&i)+1);

	return i;
}

#endif
