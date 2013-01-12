#ifndef NRG_UTIL_H
#define NRG_UTIL_H

namespace nrg {

template<class T, uint16_t type>
struct EntityHelper : nrg::Entity {
	virtual nrg::Entity* clone(){ return new T(*static_cast<T*>(this)); }
	virtual uint16_t getType() const { return type; }
};

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

}

#endif

