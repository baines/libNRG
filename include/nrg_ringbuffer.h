#ifndef NRG_RING_BUFFER_H
#define NRG_RING_BUFFER_H
#include "nrg_core.h"
#include <array>
#include <iterator>

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
			if(idx != buff->end_idx) idx = (idx + 1) % buff->data.size();
			return *this;
		}
		
		const iterator& operator--(void){
			if(idx != buff->start_idx) --idx;
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
	
	T& push(T&& t){
		data[end_idx] = t;
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
