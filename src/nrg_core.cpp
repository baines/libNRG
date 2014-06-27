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

#include "nrg_core.h"

using namespace nrg;

nrg::Version nrg::getLibVersion(void){
	return Version(0, 0, 0);
}

bool Version::operator>(const Version& v){
	return (v_major == v.v_major) ? ((v_minor == v.v_minor) 
	? v_patch > v.v_patch : v_minor > v.v_minor) : v_major > v.v_major;
}

bool Version::operator<(const Version& v){
	return (v_major == v.v_major) ? ((v_minor == v.v_minor) 
	? v_patch < v.v_patch : v_minor < v.v_minor) : v_major < v.v_major;
}

bool Version::operator>=(const Version& v){
	return !(*this < v);
}

bool Version::operator<=(const Version& v){
	return !(*this > v);
}

bool Version::operator==(const Version& v){
	return v_major == v.v_major && v_minor == v.v_minor && v_patch == v.v_patch;
}

bool Version::operator!=(const Version& v){
	return !(*this == v);
}

