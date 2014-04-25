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

template<class T, class E = void>
struct Codec {
	size_t encode(Packet& p, const T& data){
		p.write(data);
		return sizeof(data);
	}
	size_t decode(Packet& p, T& data){
		if(p.remaining() >= sizeof(data)){
			p.read(data);
			return sizeof(data);
		} else {
			return 0;
		}
	}
	template<class New>
	struct rebind {
		typedef Codec<New> type;
	};
};

template<class T>
struct Codec<T, typename enable_if<has_encode_decode<T>::value>::type> {
	size_t encode(Packet& p, const T& data){
		return data.encode(p);
	}
	size_t decode(Packet& p, T& data){
		return data.decode(p);
	}
	template<class New>
	struct rebind {
		typedef Codec<New> type;
	};

};

template<size_t len>
struct Codec<char[len]> {
	size_t encode(Packet& p, const char (&data)[len]){
		p.writeArray(data, len);
		return len;
	}
	size_t decode(Packet& p, char (&data)[len]){
		if(p.remaining() >= len){
			p.readArray(&data, len);
			return len;
		} else {
			return 0;
		}
	}
};

template<>
struct Codec<std::string> {
	size_t encode(Packet& p, const std::string& str){
		size_t ret = UVarint(str.length()).encode(p);
		p.writeArray(str.c_str(), str.length());
		return ret + str.length();
	}
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
