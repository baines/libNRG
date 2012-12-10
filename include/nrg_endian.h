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
	static const int one = 1;
	static const char tst = *reinterpret_cast<const char*>(&one);
	if(tst == 0){ // host is big endian
		return data;
	} else {
		return swapbytes(data);
	}
}

template<typename T>
static inline T hton(const T& data){
	static const int one = 1;
	static const char tst = *reinterpret_cast<const char*>(&one);
	if(tst == 0){ // host is big endian
		return data;
	} else {
		return swapbytes(data);
	}
}

}
#endif

