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
 * Library versioning stuff
 */
#ifndef NRG_VERSION_H_
#define NRG_VERSION_H_
#include "nrg_core.h"
namespace nrg {

/** @brief A class to store the version of the library. */
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

/** @brief Retuns the version of LibNRG in use. */
Version getLibVersion(void);

/** @brief Retuns true if @p v is compatible with this version of LibNRG. */
bool isVersionCompatible(const Version& v);

}

#endif

