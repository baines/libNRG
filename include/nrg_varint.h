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
 *  Template implementation of variable-length integers, following the same format as Google's Protobufs
 */
#ifndef NRG_VARINT_H
#define NRG_VARINT_H
#include "nrg_core.h"
#include <type_traits>

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

	while(i < varint_bytes<T>::value && (byte & 0x80)){
		if(!p.remaining()) return 0;
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

/** Default undeclared template instance to cause a compilation error for non-integer types */
template<class T, class = void>
struct TVarint;

/** Varint template specialisation for unsigned types */
template<class T>
struct TVarint<T, typename std::enable_if<std::is_unsigned<T>::value>::type> {

	TVarint() : data(0){}
	TVarint(const T& t) : data(t){}

	size_t requiredBytes(void) const {
		size_t ret = 1;
		T tmp = data;
		while(tmp >>= 7) ++ret;
		return ret;
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

/** Varint template specialisation for signed types that requre zigzagging */
template<class T>
struct TVarint<T, typename std::enable_if<std::is_signed<T>::value>::type> {

	TVarint() : data(0){}
	TVarint(const T& t) : data(t){}

	size_t requiredBytes(void) const {
		size_t ret = 1;
		typename std::make_unsigned<T>::type tmp = detail::varint_zigzag(data);
		while(tmp >>= 7) ++ret;
		return ret;
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

/** Typedef for unsigned varints */
typedef TVarint<uint64_t> UVarint;
/** Typedef for signed varints */
typedef TVarint<int64_t> SVarint;

}

#endif
