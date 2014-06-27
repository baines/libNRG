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
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "nrg_status.h"

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

namespace nrg {

struct Version {
	uint32_t v_major, v_minor, v_patch;
	
	Version(uint32_t a, uint32_t b, uint32_t c)
	: v_major(a)
	, v_minor(b)
	, v_patch(c){
	
	}

	bool operator>(const Version&);
	bool operator<(const Version&);
	bool operator>=(const Version&);
	bool operator<=(const Version&);
	bool operator==(const Version&);
	bool operator!=(const Version&);
};

Version getLibVersion(void);

}

#endif
