#include "nrg_snapshot.h"
#include <climits>
#include <cassert>
#include <numeric>
#include <set>

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

void Snapshot::addEntity(Entity* e){
	std::vector<FieldBase*> fields;
	EntityInfo info;
	uint8_t bits = 0;

	info.id = e->getID();
	info.type = e->getType();
	e->getFields(fields);

	field_data.write16(info.id);
	field_data.write16(info.type);
	
	for(unsigned int i = 0; i < fields.size(); ++i){
		if(fields[i]->wasUpdated()){
			bits |= 1 << (MAX_BYTE_SHIFTS - (i & MAX_BYTE_SHIFTS));
		}
		if((i < fields.size()-1) && (i & MAX_BYTE_SHIFTS) == MAX_BYTE_SHIFTS){
			field_data.write8(bits);
			bits = 0;
		}
	}
	field_data.write8(bits);
	bits = 0;

	info.start = field_data.tell();

	for(std::vector<FieldBase*>::const_iterator i = fields.begin(), 
	j = fields.end(); i!=j; ++i){
		if((*i)->wasUpdated()){
			info.field_sizes.push_back((*i)->getSize());
			(*i)->writeToPacket(field_data);
		} else {
			info.field_sizes.push_back(0);
		}
	}

	stored_entities[info.id] = info;
}

typedef std::map<uint16_t, Snapshot::EntityInfo>::const_iterator EInf_it;

bool Snapshot::merge(const Snapshot& other){
	Snapshot copy = *this, other_copy = other, *newer, *older;
	uint8_t bits = 0;

	if(other.id == ((id + 1) & USHRT_MAX)){
		newer = &other_copy;
		older = &copy;
	} else if(id == ((other.id + 1) & USHRT_MAX)){
		newer = &copy;
		older = &other_copy;
	} else {
		return false;
	}
	
	copy.field_data.seek(0, SEEK_SET);
	other_copy.field_data.seek(0, SEEK_SET);
	stored_entities.clear();
	field_data.reset();
	id = newer->id;

	std::set<uint16_t> keys;
	for(EInf_it i = older->stored_entities.begin(), j = older->stored_entities.end(); i!=j; ++i)
		keys.insert(i->first);
	for(EInf_it i = newer->stored_entities.begin(), j = newer->stored_entities.end(); i!=j; ++i)
		keys.insert(i->first);

	for(std::set<uint16_t>::const_iterator i = keys.begin(), j = keys.end(); i!=j; ++i){
		const EntityInfo *newit = NULL, *oldit = NULL;
		EntityInfo info;
		EInf_it e;
		if((e = newer->stored_entities.find(*i)) != newer->stored_entities.end()) newit = &e->second;
		if((e = older->stored_entities.find(*i)) != older->stored_entities.end()) oldit = &e->second;
		
		int sz = newit == NULL ? oldit->field_sizes.size() : newit->field_sizes.size();
		for(int x = 0; x < sz; ++x){
			if((newit && newit->field_sizes[x] != 0) || (oldit && oldit->field_sizes[x] != 0)){
				bits |= 1 << (MAX_BYTE_SHIFTS - (x & MAX_BYTE_SHIFTS));
			}
			if((x < sz-1) && (x & MAX_BYTE_SHIFTS) == MAX_BYTE_SHIFTS){
				field_data.write8(bits);
				bits = 0;
			}
		}
		field_data.write8(bits);
		bits = 0;

		info.id = *i;
		info.type = newit == NULL ? oldit->type : newit->type;
		info.start = field_data.tell();

		if(newit != NULL && oldit != NULL){
			assert(newit->field_sizes.size() == oldit->field_sizes.size());

			for(int x = 0; x < sz; ++x){
				if(newit->field_sizes[x] != 0){
					info.field_sizes.push_back(newit->field_sizes[x]);
					field_data.writeArray(newer->field_data.getPointer()
					                    + newit->start, newit->field_sizes[x]);
				} else if(oldit->field_sizes[x] != 0){
					info.field_sizes.push_back(oldit->field_sizes[x]);
					field_data.writeArray(older->field_data.getPointer()
					                    + oldit->start, oldit->field_sizes[x]);
				} else {
					info.field_sizes.push_back(0);
				}
			}
		} else {
			const EntityInfo* eptr;
			Snapshot* sptr;
			if(newit){
				eptr = newit;
				sptr = newer;
			} else {
				eptr = oldit;
				sptr = older;
			}
			info.field_sizes = eptr->field_sizes;
			field_data.writeArray(
				sptr->field_data.getPointer() + eptr->start, 
				std::accumulate(eptr->field_sizes.begin(), eptr->field_sizes.end(), 0)
			);
		}
		stored_entities[*i] = info;
	}

	return true;
}

void Snapshot::resetAndIncrement(){
	stored_entities.clear();
	field_data.reset();
	id = ((id + 1) & USHRT_MAX);
}
