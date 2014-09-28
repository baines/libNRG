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
/** @file
 *  Error-handling functionality
 */
#ifndef NRG_STATUS_H
#define NRG_STATUS_H
#include <errno.h>
#include <string.h>
#include <type_traits>

namespace nrg {

/** Class to wrap system and internal errors */
struct Status {
	/** Default Constructor, creates a no-error status */
	Status() : Status(true){}

	/** Constructor with the given error-status and errno for system-errors */
	Status(bool ok, int err = 0)
	: type(ok ? Ok : SystemError)
	, sys_errno(ok ? 0 : err)
	, desc(ok ? "No error." : "System call returned error."){
	
	}

	/** Constructor for custom errors, with the specified error string */
	Status(const char* custom)
	: type(InternalError)
	, sys_errno(0)
	, desc(custom){
	
	}
	
	/** Enumeration for error type */
	enum : int {
		Ok,
		InternalError,
		SystemError
	} type;
	
	/** The associated errno for the error, or 0 if not a system error */
	int sys_errno;
	
	/** Statically-allocated description of the error */
	const char* desc;
	
	/** bool conversion operator */
	operator bool() const {
		return type == Ok;
	}
};

/** Derived class for non-error statuses */
struct StatusOK : Status {
	StatusOK() : Status(true){}
};

/** Derived class for system errors */
struct StatusErr : Status {
	StatusErr(int e = errno) : Status(false, e){}
};

/** Portability wrapper for GNU and XSI strerror_r versions. */
static inline const char* strerr_r(int eno, char* buf, size_t sz){
#ifdef _WIN32
	strerror_s(buf, sz, eno);
	return buf;
#else
	auto r = strerror_r(eno, buf, sz);
	
	if(std::is_same<decltype(r), int>::value){
		return buf;
	} else {
		return r;
	}
#endif
}

}

#endif
