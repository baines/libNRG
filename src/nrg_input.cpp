#include "nrg_input.h"
#include "nrg_config.h"
#include <climits>

namespace nrg {
	Input null_input;
}

using namespace nrg;

void Input::markUpdated(){

}

bool Input::readFromPacket(Packet& p){
	for(FieldBase* f = getFirstField(); f; f = f->getNextField()){
		if(f->readFromPacket(p) == 0) return false;
		f->shiftData();
	}
	return true;
}

void Input::writeToPacket(Packet& p) const {
	for(FieldBase* f = getFirstField(); f; f = f->getNextField()){
		f->writeToPacket(p);
	}
}
