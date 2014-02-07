#include "nrg_snapshot.h"
#include "nrg_bit_io.h"
#include <climits>
#include <cassert>
#include <numeric>
#include <set>

using namespace nrg;

namespace {
	typedef std::map<uint16_t, Snapshot::EntityData>::iterator EDat_it;
	typedef std::map<uint16_t, Snapshot::EntityData>::const_iterator EDat_cit;
	static const size_t MAX_BYTE_SHIFTS = 7;
}

Snapshot::Snapshot() 
: id(-1)
, edata(){

}

Snapshot::Snapshot(uint16_t id)
: id(id)
, edata(){

}

void Snapshot::addEntity(Entity* e){
	bool merge = edata.find(e->getID()) != edata.end();

	EntityData& ed = edata[e->getID()];
	ed.id = e->getID();
	ed.type = e->getType();

	if(merge){
		buffer.reset().writeArray(ed.field_data.getBasePointer(), ed.field_data.size());
		ed.field_data.reset();
	}
	
	size_t s = e->getNumFields();
	if(ed.field_sizes.size() != s) ed.field_sizes.resize(s);

	FieldBase* f = e->getFirstField();
	for(size_t i = 0; i < s; ++i){
		if(f->wasUpdated()){
			ed.field_sizes[i] = f->writeToPacket(ed.field_data);
		} else if(merge && ed.field_sizes[i] != 0){
			ed.field_data.writeArray(
				buffer.getBasePointer() + ed.getFieldOffset(i), ed.field_sizes[i]
			);
		}
		f = f->getNextField();
	}
}


void Snapshot::removeEntityById(uint16_t id){
	EDat_it it = edata.find(id);
	if(it != edata.end()) edata.erase(it);
}

void Snapshot::reset(){
	edata.clear();
	id = -1;
}

void Snapshot::writeToPacket(Packet& p) const {
	size_t total_bytes = 0;
	for(EDat_cit i = edata.begin(), j = edata.end(); i!=j; ++i){
		total_bytes += i->second.getTotalBytes();
	}
	p.write32(total_bytes);

	for(EDat_cit i = edata.begin(), j = edata.end(); i!=j; ++i){
		const EntityData& ed = i->second;
		p.write16(ed.id);
		p.write16(ed.type);
	
		BitWriter(p).writeFunc(ed.field_sizes.size(), [&](int x){
			return ed.field_sizes[x] != 0;
		});
		
		p.writeArray(ed.field_data.getBasePointer(), ed.field_data.size());	
	}
}

off_t Snapshot::EntityData::getFieldOffset(int num) const {
	return std::accumulate(field_sizes.begin(), field_sizes.begin() + num, 0);
}

size_t Snapshot::EntityData::getTotalBytes() const {
	return std::accumulate(field_sizes.begin(), field_sizes.end(), 0) +
		2*sizeof(uint16_t) + ((field_sizes.size()-1)/8)+1;
}

bool ClientSnapshot::readFromPacket(Packet& p){
	uint32_t num_bytes = 0;
	p.read32(num_bytes);

	if(p.remaining() >= num_bytes){
		if(num_bytes > 0) data.writeArray(p.getPointer(), num_bytes);
		return true;
	} else {
		return false;
	}
}

void ClientSnapshot::applyUpdate(std::vector<Entity*>& entities, 
const std::map<uint16_t, Entity*>& entity_types, EventQueue& eq){

	data.seek(0, SEEK_SET);

	while(data.remaining()){
		uint16_t eid = 0, etype = 0;
		uint8_t delete_test = 0;

		data.read16(eid).read16(etype).read8(delete_test);
		
		if(delete_test == 0){
			// special case - delete entity
			if(entities.size() > eid && entities[eid] != NULL){
				EntityEvent e = { ENTITY_DESTROYED, eid, etype, entities[eid] };
				eq.pushEvent(e);

				delete entities[eid];
				entities[eid] = NULL;
			}
			
			continue;
		} else {
			data.seek(-1, SEEK_CUR);
		}

		if(entities.size() <= eid){
			entities.resize(eid+1, NULL);
		}

		if(entities[eid] == NULL){
			std::map<uint16_t, Entity*>::const_iterator i = entity_types.find(etype);
			assert(i != entity_types.end() && "Entity type not registered");
			entities[eid] = i->second->clone();
			EntityEvent e = { ENTITY_CREATED, eid, etype, entities[eid] };
			eq.pushEvent(e);
		}
		
		size_t num_fields = entities[eid]->getNumFields();
		int num_field_bytes = ((num_fields-1)/8)+1;
		uint8_t* bytes = new uint8_t[num_field_bytes]();
		
		for(int i = 0; i < num_field_bytes; ++i){
			data.read8(bytes[i]);
		}
	
		EntityEvent e = { ENTITY_UPDATED, eid, etype, entities[eid] };
		eq.pushEvent(e);

		entities[eid]->markUpdated();

		FieldBase* f = entities[eid]->getFirstField();
		for(unsigned int i = 0; i < num_fields; ++i){
			if(bytes[i/8] & (1 << (MAX_BYTE_SHIFTS - (i & MAX_BYTE_SHIFTS)))){
				f->readFromPacket(data);
				f->setUpdated(true);
			}
			f = f->getNextField();
		}

		delete [] bytes;
	}
	
	data.seek(0, SEEK_SET);
}

void ClientSnapshot::reset(){
	data.reset();
}

