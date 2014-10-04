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
using namespace nrg;

void os::autoinit(void){
	os::init();
}

#if defined(__linux)

#include <random>
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

uint64_t os::random(){
	uint32_t halves[2];
	
	int fd = open("/dev/urandom", O_RDONLY);
	const ssize_t sz = sizeof(halves);
	
	if(fd < 0 || read(fd, halves, sz) < sz){
		//fallback if /dev/urandom is unavailable;
		fprintf(stderr, "Warning: Can't generate good randomness.");
		
		uint64_t us = os::microseconds();
		uint32_t a = us, b = uint64_t(us >> 32ULL), c = getpid(), tmp = a, d = rand_r(&tmp);
		std::seed_seq({ a, b, c, d }).generate(halves, halves+2);
	}
	
	if(fd > 0) close(fd);

	return uint64_t(halves[0]) << 32 | halves[1];
}

#elif defined(_WIN32)
#define _CRT_RAND_S
#include "nrg_os.h"
#include <random>
#include <stdlib.h>

using namespace nrg;

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

uint64_t os::random(){
	uint32_t halves[2];
	
	if(rand_s(halves) != 0 || rand_s(halves+1) != 0){
		//fallback if /dev/urandom is unavailable;
		fprintf(stderr, "Warning: Can't generate good randomness.");
		
		uint64_t us = os::microseconds();
		uint32_t a = us, b = uint64_t(us >> 32ULL), c = getpid(), tmp = a, d = rand_r(&tmp);
		std::seed_seq({ a, b, c, d }).generate(halves, halves+2);
	}

	return uint64_t(halves[0]) << 32 | halves[1];
}

#endif
