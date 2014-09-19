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
#ifndef NRG_CORE_H
#define NRG_CORE_H
#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#include "windows.h"
	#include "winsock2.h"
	#include "ws2tcpip.h"
	typedef int socklen_t;
#else
	#include <unistd.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <sys/time.h>
	#include <netdb.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
#endif
#include "stdint.h"
#include "nrg_status.h"

#ifndef INET6_ADDRSTRLEN
#define INET6_ADDRSTRLEN 46
#endif

// adapted from http://gcc.gnu.org/wiki/Visibility
#if defined _WIN32 || defined __CYGWIN__
	#ifdef BUILDING_NRG
		#define NRG_PUBLIC __declspec(dllexport)
	#else
		#define NRG_PUBLIC __declspec(dllimport)
	#endif
	#define NRG_PRIVATE
#elif __GNUC__ >= 4
	#define NRG_PUBLIC  __attribute__ ((visibility ("default")))
	#define NRG_PRIVATE __attribute__ ((visibility ("hidden")))
#else
	#define NRG_PUBLIC
	#define NRG_PRIVATE
#endif

#ifdef DEBUG
	#define NRG_DEBUG(fmt, ...) do { fprintf(stderr, fmt, __VA_ARGS__); } while (0)
#else
	#define NRG_DEBUG(fmt, ...) do { } while(0)
#endif

namespace nrg {

struct Version {
	uint16_t v_major, v_minor, v_patch;
	
	Version()
	: v_major(0)
	, v_minor(0)
	, v_patch(0){
	
	}
	
	Version(uint16_t a, uint16_t b, uint16_t c)
	: v_major(a)
	, v_minor(b)
	, v_patch(c){
	
	}

	bool operator>(const Version&) const ;
	bool operator<(const Version&) const ;
	bool operator>=(const Version&) const ;
	bool operator<=(const Version&) const ;
	bool operator==(const Version&) const ;
	bool operator!=(const Version&) const ;
};

Version getLibVersion(void);

bool isVersionCompatible(const Version& v);

}

#endif
