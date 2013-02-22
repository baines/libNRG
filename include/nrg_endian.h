#ifndef NRG_ENDIAN_H
#define NRG_ENDIAN_H
#include <endian.h>
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

template<typename T>
static inline T ntoh(const T& data){
#ifdef NRG_BIG_ENDIAN
	return data;
#else
	return swapbytes(data);
#endif
}

template<typename T>
static inline T hton(const T& data){
	return nrg::ntoh(data);
}

}
#endif

