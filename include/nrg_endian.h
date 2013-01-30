#ifndef NRG_ENDIAN_H
#define NRG_ENDIAN_H
/* adapted from http://stackoverflow.com/questions/809902/64-bit-ntohl-in-c */

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
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	return data;
#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	return swapbytes(data);
#else
	#error BYTE_ORDER is not defined!
#endif
}

template<typename T>
static inline T hton(const T& data){
	return nrg::ntoh(data);
}

}
#endif

