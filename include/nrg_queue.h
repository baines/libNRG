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
 *  A simple queue implementation
 */
#include "nrg_core.h"
#include <vector>
#include <utility>
#include <stdexcept>

namespace nrg {

/** Simple queue class built on a std::vector */
template<class T>
class Queue {
public:
	/** Construct a Queue with initial size \p sz */
	Queue(size_t sz) : data(sz), tail(0), head(0){}
	
	/** Push \p t onto the back of the queue */
	void push(const T& t){
		data[tail] = t;
		tail = (tail + 1) % data.size();
		
		if(tail == head) expand(); 
	}
	
	/** Remove the head of the queue and return it, don't call this if the queue is empty or all hell will break loose */
	T pop(void){
		//if(empty()) throw std::underflow_error(__PRETTY_FUNCTION__);
		
		T& t = data[head];
		head = (head + 1) % data.size();
		return std::move(t);
	}
	
	/** Clears the queue */
	void clear(){
		head = tail = 0;
	}
	
	/** Returns the size of the queue */
	size_t size(){
		return tail >= head ? tail - head : data.size() - (head - tail);
	}
	
	/** Returns true if the queue is empty */
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

