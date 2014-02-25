#include "nrg_input.h"
#include "nrg_config.h"
#include <climits>

using namespace nrg;

void InputBase::markUpdated(bool b){

}

bool InputBase::readFromPacket(Packet& p){
	for(FieldBase* f = getFirstField(); f; f = f->getNextField()){
		if(f->readFromPacket(p) == 0) return false;
		f->shiftData();
	}
	return true;
}

void InputBase::writeToPacket(Packet& p) const {
	for(FieldBase* f = getFirstField(); f; f = f->getNextField()){
		f->writeToPacket(p);
	}
}
