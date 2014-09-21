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
 *  Operating-system specific functions
 */
#ifndef NRG_OS_H
#define NRG_OS_H
#include "nrg_core.h"

namespace nrg {
namespace os {

	/** Perform OS initialisation routines */
	Status init(void);

	/** Perform OS uninitiasation routines */
	void uninit(void);

	/** Get a monotonic representation of time in microseconds */
	uint64_t microseconds();
	
	/** Get a monotonic representation of time in milliseconds */
	inline uint64_t milliseconds(){
		return microseconds() / 1000;
	}

	/** Get a monotonic representation of time in seconds */
	inline uint64_t seconds(){
		return milliseconds() / 1000;
	}
	
	/** Returns a reasonably random number, not guaranteed to be cryptographically secure */
	uint32_t random();

}
}

#endif
