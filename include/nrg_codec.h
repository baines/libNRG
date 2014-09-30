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
 *  Contains the Codec class, which encodes and decodes types to and from Packets
 */
#ifndef NRG_CODEC_H
#define NRG_CODEC_H
#include "nrg_core.h"
#include "nrg_varint.h"
#include <string>
#include <type_traits>

namespace nrg {

namespace detail {

using namespace std;

template<class T>
class has_encode_decode {
	template<class C, C> struct S {};
	template<class X> static true_type check(S<size_t (X::*)(Packet&) const, &X::encode>*,
	                                         S<size_t (X::*)(Packet&), &X::decode>*);
	template<class X> static false_type check(...);
public:
	static const bool value = decltype(check<T>(0, 0))::value;
};

}

using std::enable_if;
using detail::has_encode_decode;

/** Encodes and Decodes any type into a Packet object */
template<class T, class E = void>
struct Codec {
	/** Encodes \p data into \p p, and retuns the number of bytes written */
	size_t encode(Packet& p, const T& data){
		p.write(data);
		return sizeof(data);
	}

	/** Decodes bytes from \p p into \p data, returns the number of bytes read or 0 on error */
	size_t decode(Packet& p, T& data){
		if(p.remaining() >= sizeof(data)){
			p.read(data);
			return sizeof(data);
		} else {
			return 0;
		}
	}
};

/** Specialisation of Codec for types that have their own \p encode and \p decode methods */
template<class T>
struct Codec<T, typename enable_if<has_encode_decode<T>::value>::type> {
	/** Calls the encode method of \p data to encode itself into the Packet \p p */
	size_t encode(Packet& p, const T& data){
		return data.encode(p);
	}

	/** Calls the decode method of \p data to decode itself from the Packet \p p */
	size_t decode(Packet& p, T& data){
		return data.decode(p);
	}
};

/** Specialisation of Codec for arrays of char */
template<size_t len>
struct Codec<char[len]> {

	/** Encodes \p data into \p p, and retuns the number of bytes written */
	size_t encode(Packet& p, const char (&data)[len]){
		p.writeArray(data, len);
		return len;
	}

	/** Decodes bytes from \p p into \p data, returns the number of bytes read or 0 on error */
	size_t decode(Packet& p, char (&data)[len]){
		if(p.remaining() >= len){
			p.readArray(&data, len);
			return len;
		} else {
			return 0;
		}
	}
};

/** Specialisation of Codec for std::string objects */
template<>
struct Codec<std::string> {

	/** Encodes \p data into \p p by prefixing the string with its length as a Varint */
	size_t encode(Packet& p, const std::string& str){
		size_t ret = UVarint(str.length()).encode(p);
		p.writeArray(str.c_str(), str.length());
		return ret + str.length();
	}

	/** Decodes bytes from \p p into \p data, returns the number of bytes read or 0 on error */
	size_t decode(Packet& p, std::string& str){
		if(!p.remaining()) return 0;

		UVarint v;
		size_t read_bytes = 0;

		if((read_bytes = v.decode(p)) == 0) return 0;
		if(p.remaining() < v.get()) return 0;

		str = std::string(reinterpret_cast<const char*>(p.getPointer()), v.get());

		p.seek(v.get(), SEEK_CUR);
		return read_bytes + v.get();
	}
};

}

#endif
