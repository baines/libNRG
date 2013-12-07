#include "nrg_core.h"
#include <vector>
#include <utility>
#include <stdexcept>

namespace nrg {

template<class T>
class Queue {
public:

	Queue(size_t sz) : data(sz), tail(0), head(0) {
	
	}
	
	void push(const T& t){
		data[tail] = t;
		tail = (tail + 1) % data.size();
		
		if(tail == head) expand(); 
	}
	
	T pop(void){
		//if(empty()) throw std::underflow_error(__PRETTY_FUNCTION__);
		
		T& t = data[head];
		head = (head + 1) % data.size();
		return std::move(t);
	}
	
	void clear(){
		head = tail = 0;
	}
	
	size_t size(){
		return tail >= head ? tail - head : data.size() - (head - tail);
	}
	
	bool empty(){
		return head == tail;
	}
	
private:

	void expand(){
		size_t oldsz = data.size();
		data.resize(oldsz * 2);
		
		for(size_t i = 0; i < oldsz; ++i){
			data[oldsz + i] = std::move(data[(head + i) % oldsz]);
		}
		
		tail = 0;
		head = oldsz;
	}

	std::vector<T> data;
	size_t tail, head;
};

}

