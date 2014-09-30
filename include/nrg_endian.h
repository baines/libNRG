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
 *  Defines functions for converting between big and little endian byte orders
 */
#ifndef NRG_ENDIAN_H
#define NRG_ENDIAN_H
/* adapted from http://stackoverflow.com/questions/809902/64-bit-ntohl-in-c */

#if defined(__BYTE_ORDER__)
	#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN
		#define NRG_BIG_ENDIAN 1
	#endif
#elif defined(BYTE_ORDER)
	#if BYTE_ORDER == BIG_ENDIAN
		#define NRG_BIG_ENDIAN 1
	#endif
#else
	#warning Assuming host is little endian.
#endif

namespace nrg {

/** Swap the byte order of \p data between big and little endianness */
template<typename T>
static inline T swapbytes(const T& data){
	T ret;
	const char* src = reinterpret_cast<const char*>(&data) + sizeof(T) - 1;
	char* dst = reinterpret_cast<char*>(&ret);

	for(int i = sizeof(T); i > 0; --i){
		*dst++ = *src--;
	}

	return ret;
}

/** Convert \p data from big endian (network byte order) to host byte order */
template<typename T>
static inline T ntoh(const T& data){
#ifdef NRG_BIG_ENDIAN
	return data;
#else
	return swapbytes(data);
#endif
}

/** Convert \p data from host byte order to big endian (network byte order */
template<typename T>
static inline T hton(const T& data){
	return nrg::ntoh(data);
}

}
#endif

