#include "nrg_snapshot.h"
#include "nrg_field_impl.h"
#include <climits>
#include <cassert>
#include <numeric>
#include <set>

static const int MAX_BYTE_SHIFTS = 7;

using namespace nrg;

Snapshot::Snapshot() : id(-1), edata(), field_data() {

}

Snapshot::Snapshot(uint16_t id) : id(id), edata(), field_data() {

}

Snapshot::Snapshot(const Snapshot& copy) : id(copy.id), edata(copy.edata),
field_data(copy.field_data){
	
}

Snapshot& Snapshot::operator=(const Snapshot& other){
	id = other.id;
	edata = other.edata;
	field_data = other.field_data;
	return *this;
}

void Snapshot::addEntity(Entity* e){
	FieldListImpl fl;
	EntityInfo info;
	uint8_t bits = 0;

	info.id = e->getID();
	info.type = e->getType();
	e->getFields(fl);
	std::vector<FieldBase*>& fields = fl.vec;

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
			info.field_sizes.push_back((*i)->writeToPacket(field_data));
		} else {
			info.field_sizes.push_back(0);
		}
	}

	edata[info.id] = info;
}

typedef std::map<uint16_t, Snapshot::EntityInfo>::const_iterator EInf_it;

bool Snapshot::merge(const Snapshot& other){
	Snapshot copy = *this, other_copy = other, *newer, *older;
	uint8_t bits = 0;

	if(id == -1 || other.id == ((id + 1) & USHRT_MAX)){
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
	edata.clear();
	field_data.reset();
	id = newer->id;

	std::set<uint16_t> keys;
	for(EInf_it i = older->edata.begin(), j = older->edata.end(); i!=j; ++i)
		keys.insert(i->first);
	for(EInf_it i = newer->edata.begin(), j = newer->edata.end(); i!=j; ++i)
		keys.insert(i->first);

	for(std::set<uint16_t>::const_iterator i = keys.begin(), j = keys.end(); i!=j; ++i){
		const EntityInfo *newit = NULL, *oldit = NULL;
		EntityInfo info;
		EInf_it e;

		if((e = newer->edata.find(*i)) != newer->edata.end()){
			newit = &e->second;
		}
		if((e = older->edata.find(*i)) != older->edata.end()){
			oldit = &e->second;
		}
		
		info.id = *i;
		info.type = newit == NULL ? oldit->type : newit->type;
		int sz = newit == NULL ? oldit->field_sizes.size() : newit->field_sizes.size();

		field_data.write16(info.id);
		field_data.write16(info.type);

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

		info.start = field_data.tell();

		const EntityInfo* eptr;
		Snapshot* sptr;

		if(newit != NULL && oldit != NULL){
			assert(newit->field_sizes.size() == oldit->field_sizes.size());

			for(int x = 0; x < sz; ++x){
				if(newit->field_sizes[x] == 0 && oldit->field_sizes[x] == 0){
					info.field_sizes.push_back(0);
				} else {
					if(newit->field_sizes[x] != 0){
						eptr = newit;
						sptr = newer;
					} else {
						eptr = oldit;
						sptr = older;
					}
					size_t fsz = eptr->field_sizes[x];
					info.field_sizes.push_back(fsz);
					field_data.writeArray(
						sptr->field_data.getPointer() + 
						eptr->getFieldOffset(x), fsz
					);
				}
			}
		} else {
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
		edata[*i] = info;
	}

	return true;
}

void Snapshot::resetAndIncrement(){
	edata.clear();
	field_data.reset();
	id = ((id + 1) & USHRT_MAX);
}

void Snapshot::writeToPacket(Packet& p){
	p.write32(field_data.size());
	p.writeArray(field_data.getBasePointer(), field_data.size());
}

bool Snapshot::readFromPacket(Packet& p){
	uint32_t num_bytes = 0;
	p.read32(num_bytes);

	if(p.remaining() >= num_bytes){
		field_data.writeArray(p.getPointer(), num_bytes);
		return true;
	} else {
		return false;
	}
}

void Snapshot::applyUpdate(std::vector<Entity*>& entities, 
const std::map<uint16_t, Entity*>& entity_types, EventQueue& eq){

	field_data.seek(0, SEEK_SET);

	while(field_data.remaining()){
		uint16_t eid = 0, etype = 0;
		field_data.read16(eid);
		field_data.read16(etype);

		if(entities.size() <= eid){
			entities.resize(eid+1);
		}

		if(entities[eid] == NULL){
			std::map<uint16_t, Entity*>::const_iterator i = entity_types.find(eid);
			assert(i != entity_types.end() && "Entity type not registered");
			entities[eid] = i->second->clone();
			EntityEvent e = { ENTITY_CREATED, eid, etype, entities[eid] };
			eq.pushEvent(e);
		}
		
		FieldListImpl fl;
		entities[eid]->getFields(fl);

		size_t num_fields = fl.vec.size();
		int num_field_bytes = ((num_fields-1)/8)+1;
		uint8_t* bytes = new uint8_t[num_field_bytes]();
		int all_zero_test = 0;
		
		for(int i = 0; i < num_field_bytes; ++i){
			field_data.read8(bytes[i]);
			all_zero_test |= bytes[i];
		}
	
		if(all_zero_test == 0){
			// special case - delete entity
			EntityEvent e = { ENTITY_DESTROYED, eid, etype, NULL };
			eq.pushEvent(e);

			delete entities[eid];
			entities[eid] = NULL;
		} else {
			EntityEvent e = { ENTITY_UPDATED, eid, etype, entities[eid] };
			eq.pushEvent(e);

			entities[eid]->markUpdated();

			for(unsigned int i = 0; i < num_fields; ++i){
				if(bytes[i/8] & (1 << (MAX_BYTE_SHIFTS - (i & MAX_BYTE_SHIFTS)))){
					fl.vec[i]->readFromPacket(field_data);
					fl.vec[i]->setUpdated(true);
				}
			}
		}
	}
	
	field_data.seek(0, SEEK_SET);
}

off_t Snapshot::EntityInfo::getFieldOffset(int num) const {
	return start + std::accumulate(field_sizes.begin(), field_sizes.begin() + num, 0);
}


