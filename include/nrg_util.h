#ifndef NRG_UTIL_H
#define NRG_UTIL_H
#include "nrg_core.h"
#include <vector>

namespace nrg {

template<class ID>
class IDAssigner {
public:
	IDAssigner(const ID& start = 0) : max_id(start){};
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
	}
private:
	std::vector<ID> reusable_ids;
	ID max_id;
};

template<size_t N>
struct min_sizeof {
	static const size_t val 
		= (N < 256U)        ? 1 // pow(2,8)
		: (N < 65536U)      ? 2 // pow(2,16)
		: (N < 4294967296U) ? 4 // pow(2,32)
		: 8
		;
};

template<int N> struct size2type {};
template<>      struct size2type<1> { typedef uint8_t  type;};
template<>      struct size2type<2> { typedef uint16_t type;};
template<>      struct size2type<4> { typedef uint32_t type;};
template<>      struct size2type<8> { typedef uint64_t type;};

}

#endif

