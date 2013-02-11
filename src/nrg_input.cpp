#include "nrg_input.h"
#include "nrg_config.h"
#include "nrg_field_impl.h"
#include <climits>

namespace nrg {
	NullInput null_input;
}

using namespace nrg;

void Input::markUpdated(){

}

bool Input::readFromPacket(Packet& p){
	FieldListImpl fl;
	getFields(fl);

	for(size_t i = 0; i < fl.size(); ++i){
		if(fl.vec[i]->readFromPacket(p) == 0) return false;
		fl.vec[i]->shiftData();
	}
	return true;
}

void Input::writeToPacket(Packet& p) {
	FieldListImpl fl;
	getFields(fl);

	for(size_t i = 0; i < fl.size(); ++i){
		fl.vec[i]->writeToPacket(p);
	}
}
