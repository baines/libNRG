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
#ifndef NRG_STATUS_H
#define NRG_STATUS_H
#include <errno.h>
#include <string.h>
#include <type_traits>

namespace nrg {

struct Status {

	Status() : Status(true){}

	Status(bool ok, int err = 0)
	: type(ok ? Ok : SystemError)
	, sys_errno(ok ? 0 : err)
	, desc(ok ? "No error." : "System call returned error."){
	
	}

	Status(const char* custom)
	: type(InternalError)
	, sys_errno(0)
	, desc(custom){
	
	}
	
	enum : int {
		Ok,
		InternalError,
		SystemError
	} type;
	int sys_errno;
	const char* desc;
	
	operator bool() const {
		return type == Ok;
	}
};

struct StatusOK : Status {
	StatusOK() : Status(true){}
};

struct StatusErr : Status {
	StatusErr(int e = errno) : Status(false, e){}
};

// portability wrapper for both GNU and XSI strerror_r versions.
static inline const char* strerr_r(int eno, char* buf, size_t sz){
	auto r = strerror_r(eno, buf, sz);
	
	if(std::is_same<decltype(r), int>::value){
		return buf;
	} else {
		return r;
	}
}

}

#endif
