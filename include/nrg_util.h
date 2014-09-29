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
 *  Various utility classes and functions
 */
#ifndef NRG_UTIL_H
#define NRG_UTIL_H
#include "nrg_core.h"
#include <vector>
#include <algorithm>
#include <functional>
#include <memory>
#include <utility>

namespace nrg {

/** @internal Implementation of std::make_unique for pre-C++14 use */
template< class T, class... Args >
std::unique_ptr<T> make_unique( Args&&... args ){
	return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

/** @internal Class to make efficient re-use of identifiers */
template<class ID>
class IDAssigner {
public:
	IDAssigner(const ID& start = 0) : max_id(start){}
	ID acquire(){
		if(reusable_ids.empty()){
			return max_id++;
		} else {
			ID id = reusable_ids.back();
			reusable_ids.pop_back();
			return id;
		}
	}
	void release(const ID& id){
		reusable_ids.push_back(id);
		std::sort(reusable_ids.begin(), reusable_ids.end(), std::greater<ID>());
	}
private:
	std::vector<ID> reusable_ids;
	ID max_id;
};

/** Client statistics interface */
struct ClientStats {

	/** Get the number of available snapshot statistics */
	virtual size_t getNumSnapshotStats() const = 0;
	
	/** Get a snapshot statistic which shows the latency at which snapshots were received, -1 means it was dropped */
	virtual int getSnapshotStat(size_t index) const = 0;
	
	/** Get the number of available interpolation statistics */
	virtual size_t getNumInterpStats() const = 0;
	
	/** Get a statistic showing how far between snapshots the client is interpolating, >1 means it is having to extrapolate data due to not receiving the next snapshot in time */
	virtual float getInterpStat(size_t index) const = 0;
	
	/** Create a <a href="https://en.wikipedia.org/wiki/Lagometer">Lagometer</a> texture from the stats */
	virtual uint8_t* toRGBATexture(uint32_t (&tex)[64*64]) const = 0;

	virtual ~ClientStats(){}
};

namespace detail {

/** @internal Template meta-programming utility to find the minimum number of bytes needed to store a number */
template<size_t N>
struct min_sizeof {
	static const size_t val 
		= (N < 256U)        ? 1 // pow(2,8)
		: (N < 65536U)      ? 2 // pow(2,16)
		: (N < 4294967296U) ? 4 // pow(2,32)
		: 8
		;
};

/** @internal Template meta-programming utility to get a type with the specified byte-width */
template<int N> struct size2type {};
template<>      struct size2type<1> { typedef uint8_t  type;};
template<>      struct size2type<2> { typedef uint16_t type;};
template<>      struct size2type<4> { typedef uint32_t type;};
template<>      struct size2type<8> { typedef uint64_t type;};

}

}

#endif

