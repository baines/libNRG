#include "nrg_input.h"
#include "nrg_config.h"
#include <climits>

using namespace nrg;
typedef std::map<int32_t, Input::Event>::iterator ev_it;

Input::Input() : input_id(0), events(){

}

Input& Input::nextFrame(){
	uint64_t next_id = input_id + 1;

	for(ev_it i = events.begin(), j = events.end(); i!=j; /**/){
		if(i->second.state_added <= (next_id - NRG_NUM_PAST_SNAPSHOTS)){
			events.erase(i++);
		} else {
			++i;
		}
	}
	
	input_id = next_id & UINT_MAX;
	return *this;
}

void Input::addEvent(int32_t key, int32_t value){
	Event e = { value, input_id };
	events.insert(std::pair<int32_t, Event>(key, e));
}

void Input::acknowledge(uint32_t id){
	for(ev_it i = events.begin(), j = events.end(); i!=j;){
		bool remove = false;
		for(int x = 0; x < NRG_NUM_PAST_SNAPSHOTS; ++x){
			if(i->second.state_added == (id-x)){
				remove = true;
				break;
			}
		}
		if(remove){
			events.erase(i++);
		} else {
			++i;
		}
	}
}

void Input::writeToPacket(Packet& p){
	p.write32(input_id);
	for(ev_it i = events.begin(), j = events.end(); i!=j; ++i){
		p.write32(i->first).write32(i->second.value);
	}
}

void Input::readFromPacket(Packet& p){
	p.read32(input_id);
	while(p.remaining() >= 2 * sizeof(int32_t)){
		uint32_t k, v;
		p.read32(k).read32(v);
		Event e = { v, input_id };
		events.insert(std::pair<int32_t, Event>(k, e));
	}
}


