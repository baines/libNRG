#include "nrg_snapshot.h"
#include <climits>
static const int MAX_BYTE_SHIFTS = 7;

using namespace nrg;

Snapshot::Snapshot() : id(-1), stored_entities(), field_data() {

}

Snapshot::Snapshot(uint16_t id) : id(id), stored_entities(), field_data() {

}

Snapshot::Snapshot(const Snapshot& copy) : id(copy.id), stored_entities(copy.stored_entities),
field_data(copy.field_data){
	
}

Snapshot& Snapshot::operator=(const Snapshot& other){
	id = other.id;
	stored_entities = other.stored_entities;
	field_data = other.field_data;
	return *this;
}

void Snapshot::addEntity(const Entity* e){
	std::vector<FieldBase*> fields;
	EntityInfo info;
	uint8_t bits = 0;

	e->getFields(fields);
	field_data.write16(e->getID());
	field_data.write16(e->getType());
	
	for(unsigned int i = 0; i < fields.size(); ++i){
		if(fields[i]->wasUpdated()){
			bits |= 1 << (MAX_BYTE_SHIFTS - (i & MAX_BYTE_SHIFTS));
		}
		if((i & MAX_BYTE_SHIFTS) == MAX_BYTE_SHIFTS){
			field_data.write8(bits);
			bits = 0;
		}
	}

	info.id = e->getID();
	info.start = field_data.tell();

	for(std::vector<FieldBase*>::const_iterator i = fields.begin(), j = fields.end(); i!=j; ++i){
		if((*i)->wasUpdated()){
			info.field_sizes.push_back((*i)->getSize());
			(*i)->writeToPacket(field_data);
		}
	}

	stored_entities.insert(info);
}

bool Snapshot::merge(const Snapshot& other){
	Snapshot copy = *this, other_copy = other, *newer, *older;

	if(other.id == ((id + 1) & USHRT_MAX)){
		newer = &other_copy;
		older = &copy;
	} else if(id == ((other.id + 1) & USHRT_MAX)){
		newer = &copy;
		older = &other_copy;
	} else {
		return false;
	}
	
	stored_entities.clear();
	field_data.reset();
	id = newer->id;	

	for(std::set<EntityInfo>::const_iterator i = newer->stored_entities.begin(),
	j = newer->stored_entities.end(); i != j; ++i){
		if(older->stored_entities.find(*i) != older->stored_entities.end()){
			//TODO
		}
	}

	return true;
}

void Snapshot::resetAndIncrement(){
	stored_entities.clear();
	field_data.reset();
	id = ((id + 1) & USHRT_MAX);
}
