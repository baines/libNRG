#ifndef NRG_VARINT_H
#define NRG_VARINT_H
#include "nrg_core.h"
#include <type_traits>
#include <iostream>

namespace nrg {

using namespace std;

namespace detail {

template<class T>
struct varint_bytes {
	static const size_t value = 1 + ((sizeof(T) * 8) - 1) / 7;
};

template<class T, class = typename enable_if<is_signed<T>::value>::type>
typename make_unsigned<T>::type varint_zigzag(T i){
	return (i << 1) ^ (i >> numeric_limits<T>::digits);
}

template<class T, class = typename enable_if<is_unsigned<T>::value>::type>
typename make_signed<T>::type varint_zagzig(T i){
	return (i >> 1) ^ (-(i & 1));
}

template<typename T>
typename enable_if<is_unsigned<T>::value, size_t>::type varint_decode(Packet& p, T& ans){
	ans = 0;
	size_t i = 0;
	uint8_t byte = 0x80;
	
	while(i < varint_bytes<T>::value && p.remaining() && (byte & 0x80)){
		p.read8(byte);
		ans |= ((byte & 0x7F) << (i++ * 7));
	}
	
	return i;
}

template<typename T>
typename enable_if<is_unsigned<T>::value, size_t>::type varint_encode(Packet& p, T num){
	uint8_t enc[varint_bytes<T>::value];
	size_t i = 0;
	
	while(enc[i++] = (num & 0x7F), num >>= 7){
		enc[i-1] |= 0x80;
	}
	
	p.writeArray(enc, i);
	return i;
}

}

template<class T, class = void>
struct TVarint;

template<class T>
struct TVarint<T, typename std::enable_if<std::is_unsigned<T>::value>::type> {

	TVarint() : data(0){}
	TVarint(const T& t) : data(t){}
	
	size_t requiredBytes(void) const {
		return (data == 0) ? 1 : (log2(data)/7)+1;
	}

	size_t encode(Packet& p) const {
		return detail::varint_encode(p, data);	
	}
	
	size_t decode(Packet& p){
		return detail::varint_decode(p, data);
	}
	
	T quickDecode(Packet& p){
		T t = 0;
		detail::varint_decode(p, t);
		return t;
	}

	operator T () const { return data; }
	T get() const { return data; }
	
	T data;
};

template<class T>
struct TVarint<T, typename std::enable_if<std::is_signed<T>::value>::type> {

	TVarint() : data(0){}
	TVarint(const T& t) : data(t){}
	
	size_t requiredBytes(void) const {
		typename std::make_unsigned<T>::type udata = detail::varint_zigzag(data);
		return !udata ? 1 : (log2(udata)/7)+1;
	}

	size_t encode(Packet& p) const {
		return detail::varint_encode(p, detail::varint_zigzag(data));
	}
	
	size_t decode(Packet& p){
		typename std::make_unsigned<T>::type udata = 0;
		size_t r = detail::varint_decode(p, udata);
		data = detail::varint_zagzig(udata);
		return r;
	}
	
	T quickDecode(Packet& p){
		typename std::make_unsigned<T>::type udata = 0;
		detail::varint_decode(p, udata);
		return detail::varint_zagzig(udata);
	}
	
	operator T () const { return data; }
	T get() const { return data; }

	T data;
};

typedef TVarint<uint64_t> UVarint;
typedef TVarint<int64_t> SVarint;

}

#endif
