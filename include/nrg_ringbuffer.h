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
#ifndef NRG_RING_BUFFER_H
#define NRG_RING_BUFFER_H
#include "nrg_core.h"
#include <array>
#include <iterator>
#include <utility>

namespace nrg {

/*XXX: This implementation only has the bare minimum functionality that is
       required internally. Also it only holds N-1 elements which is kinda broken.
*/
template<class T, size_t N>
class RingBuffer {
public:
	RingBuffer() : data(), start_idx(0), end_idx(0){}
		
	class iterator : public std::iterator<std::forward_iterator_tag, const T> {
	public:
		iterator(const RingBuffer* const rb, size_t i) : buff(rb), idx(i){}
		
		bool operator<(const iterator& other) const {
			size_t dist_a = calcDist(*this), dist_b = calcDist(other);
			return dist_a < dist_b;
		}
		
		bool operator==(const iterator& other) const {
			return idx == other.idx;
		}
		
		bool operator!=(const iterator& other) const {
			return idx != other.idx;
		}
		
		const iterator& operator++(void){
			if(idx != (ssize_t)buff->end_idx) idx = (idx + 1) % buff->data.size();
			return *this;
		}
		
		const iterator& operator--(void){
			if(idx != (ssize_t)buff->start_idx) --idx;
			if(idx < 0) idx = buff->data.size()-1;
			return *this;
		}
		
		const T* operator->() const {
			if(idx == buff->end_idx){
				return nullptr;
			} else {
				return &buff->data[idx];
			}
		}
		
		const T& operator*() const {
			return buff->data[idx];
		}
	private:
		size_t calcDist(const iterator& i){
			if(i.idx > i.buff.start_idx){
				return i.idx - i.buff->start_idx;
			} else {
				return i.buff->size() - (i.buff->start_idx - i.idx);
			}
		}
		const RingBuffer* buff;
		ssize_t idx;
	};
	
	T& next(void){
		T& ret = data[end_idx];
		end_idx = (end_idx + 1) % data.size();
		if(end_idx == start_idx){
			start_idx = (start_idx + 1) % data.size();
		}
		return ret;
	}
	
	iterator begin() const {
		return iterator(this, start_idx);
	}
	
	iterator end() const {
		return iterator(this, end_idx);
	}
	
	std::reverse_iterator<iterator> rbegin() const {
		return std::reverse_iterator<iterator>(end());
	}
	
	std::reverse_iterator<iterator> rend() const {
		return std::reverse_iterator<iterator>(begin());
	}
	
private:
	std::array<T, N> data;
	size_t start_idx, end_idx;
};

}

#endif
