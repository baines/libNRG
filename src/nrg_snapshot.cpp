#include "nrg_snapshot.h"
#include "nrg_bit_io.h"
#include "nrg_varint.h"
#include <climits>
#include <cassert>
#include <numeric>
#include <set>

using namespace nrg;

namespace {
	typedef std::map<uint16_t, Snapshot::EntityData>::iterator EDat_it;
	typedef std::map<uint16_t, Snapshot::EntityData>::const_iterator EDat_cit;
	
	enum {
		SNAPFLAG_DEL_SECTION  = 0x80,
		SNAPFLAG_FULL_SECTION = 0x40,
		SNAPFLAG_UPD_SECTION  = 0x20
	};
}

Snapshot::Snapshot() 
: id(0)
, edata(){

}

Snapshot::Snapshot(uint16_t id)
: id(id)
, edata(){

}

void Snapshot::addEntity(Entity* e){
	bool merge = edata.find(e->getID()) != edata.end();

	EntityData& ed = edata[e->getID()];
	ed.eid = e->getID();
	ed.etype = e->getType();

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
		} else {
			assert(merge && ed.field_sizes[i] != 0);
			
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
	id = 0;
}

void Snapshot::writeToPacket(Packet& p) const {
	p.write8(SNAPFLAG_FULL_SECTION);
	UVarint(edata.size()).encode(p);

	for(EDat_cit i = edata.begin(), j = edata.end(); i!=j; ++i){
		const EntityData& ed = i->second;
		UVarint(ed.eid).encode(p);
		UVarint(ed.etype).encode(p);
			
		p.writeArray(ed.field_data.getBasePointer(), ed.field_data.size());
	}
}

off_t Snapshot::EntityData::getFieldOffset(int num) const {
	return std::accumulate(field_sizes.begin(), field_sizes.begin() + num, 0);
}

size_t Snapshot::EntityData::getTotalBytes() const {
	return field_data.size() + UVarint(eid).requiredBytes() + UVarint(etype).requiredBytes();
}

//FIXME: this function should probably be merged with applyUpdate
bool ClientSnapshot::readFromPacket(Packet& p){
	data.seek(0, SEEK_SET).writeArray(p.getPointer(), p.remaining());
	return true;
}

void ClientSnapshot::applyUpdate(std::vector<Entity*>& entities, 
const std::map<uint16_t, Entity*>& entity_types, EventQueue& eq){
	data.seek(0, SEEK_SET);
	UVarint ecount(0);
	uint8_t section_bits = 0;
	data.read8(section_bits);
	
	if(section_bits & SNAPFLAG_DEL_SECTION){
		ecount.decode(data);
		for(size_t i = 0; i < ecount; ++i){
			uint16_t eid = UVarint().quickDecode(data);
			if(entities.size() > eid && entities[eid]){
				Entity*& e = entities[eid];
				eq.pushEvent(EntityEvent{ ENTITY_DESTROYED, eid, e->getType(), e });
				delete e;
				e = nullptr;
			}
		}
	}
	
	if(section_bits & SNAPFLAG_FULL_SECTION){
		ecount.decode(data);
		for(size_t i = 0; i < ecount; ++i){
			uint16_t eid = UVarint().quickDecode(data);
			uint16_t etype = UVarint().quickDecode(data);

			entities.resize(std::max<size_t>(entities.size(), eid+1), nullptr); // XXX vulnerable to memory DoS
			Entity*& e = entities[eid];
		
			if(e && e->getType() != etype){
				eq.pushEvent(EntityEvent{ ENTITY_DESTROYED, eid, e->getType(), e });
				delete e;
				e = nullptr;
			}
		
			if(!e){
				auto it = entity_types.find(etype);
				assert(it != entity_types.end());
		
				e = it->second->clone();
				e->setID(eid);
				eq.pushEvent(EntityEvent{ ENTITY_CREATED, eid, etype, e });
			}
		
			for(FieldBase* f = e->getFirstField(); f; f = f->getNextField()){
				f->readFromPacket(data);
				f->setUpdated(true);
			}
		
			eq.pushEvent(EntityEvent{ ENTITY_UPDATED, eid, e->getType(), e });
			e->markUpdated();
		}
	}
	
	if(section_bits & SNAPFLAG_UPD_SECTION){
		ecount.decode(data);
		for(size_t i = 0; i < ecount; ++i){
			uint16_t eid = UVarint().quickDecode(data);
			assert(entities.size() > eid && entities[eid]);
			Entity*& e = entities[eid];
			FieldBase* f = e->getFirstField();
			off_t read_pos = data.tell() + 1+(e->getNumFields()-1)/8;

			BitReader(data).readFunc(e->getNumFields(), [&](int x, bool set){
				if(set){
					off_t orig_pos = data.tell();
					data.seek(read_pos, SEEK_SET);
					read_pos += f->readFromPacket(data);
					f->setUpdated(true);
					data.seek(orig_pos, SEEK_SET);
				}
				f = f->getNextField();
			});
		
		    data.seek(read_pos, SEEK_SET);
		
			eq.pushEvent(EntityEvent{ ENTITY_UPDATED, eid, e->getType(), e });
			e->markUpdated();
		}
	}
	
	data.seek(0, SEEK_SET);
}

void ClientSnapshot::reset(){
	data.reset();
}

