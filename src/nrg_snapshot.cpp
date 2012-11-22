#include "nrg_snapshot.h"
#include <vector>

using namespace nrg;

Snapshot::Snapshot() : id(-1), data() {

}

Snapshot::Snapshot(uint16_t id) : id(id), data() {

}

void Snapshot::addEntity(const Entity* e){
	std::vector<FieldBase*> fields;
	uint8_t bits = 0;

	e->getFields(fields);
	data.write16(e->getID());
	data.write16(e->getType());
	
	for(int i = 0; i < fields.size(); ++i){
		if(fields[i]->wasUpdated()){
			bits |= 1 << (7 - (i & 7));
		}
		if((i & 7) == 7){
			data.write8(bits);
			bits = 0;
		}
	}

	for(std::vector<FieldBase*>::const_iterator i = fields.begin(), j = fields.end(); i!=j; ++i){
		if((*i)->wasUpdated()){
			(*i)->writeToPacket(data);
		}
	}
}
