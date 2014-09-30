/*
  LibNRG - Networking for Real-time Games

  Copyright (C) 2012-2014 Alex Baines <alex@abaines.me.uk>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include "nrg_snapshot.h"
#include "nrg_bit_io.h"
#include "nrg_varint.h"
#include <climits>
#include <cassert>
#include <numeric>
#include <set>

using namespace nrg;
using namespace std;

namespace {
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


void Snapshot::removeEntityByID(uint16_t id){
	auto it = edata.find(id);
	if(it != edata.end()) edata.erase(it);
}

void Snapshot::reset(){
	edata.clear();
	id = 0;
}

void Snapshot::writeToPacket(Packet& p) const {
	if(edata.size()){
		p.write8(SNAPFLAG_FULL_SECTION);
		UVarint(edata.size()).encode(p);

		for(auto i = edata.begin(), j = edata.end(); i!=j; ++i){
			const EntityData& ed = i->second;
			UVarint(ed.eid).encode(p);
			UVarint(ed.etype).encode(p);

			p.writeArray(ed.field_data.getBasePointer(), ed.field_data.size());
		}
	} else {
		p.write8(0);
	}
}

off_t Snapshot::EntityData::getFieldOffset(int num) const {
	return accumulate(field_sizes.begin(), field_sizes.begin() + num, 0);
}

size_t Snapshot::EntityData::getTotalBytes() const {
	return field_data.size() + UVarint(eid).requiredBytes() + UVarint(etype).requiredBytes();
}

bool ClientSnapshot::readFromPacket(Packet& data, const CSnapFunc& entity_fn){
	UVarint ecount(0);
	uint8_t section_bits = 0;
	data.read8(section_bits);

	if(section_bits & SNAPFLAG_DEL_SECTION){
		ecount.decode(data);
		for(size_t i = 0; i < ecount; ++i){
			uint16_t eid = UVarint().quickDecode(data);
			entity_fn(Action::Destroy, eid, 0);
		}
	}

	if(section_bits & SNAPFLAG_FULL_SECTION){
		ecount.decode(data);
		for(size_t i = 0; i < ecount; ++i){
			uint16_t eid = UVarint().quickDecode(data);
			uint16_t etype = UVarint().quickDecode(data);

			Entity* e = entity_fn(Action::Get, eid, 0);

			if(e && e->getType() != etype){
				entity_fn(Action::Destroy, eid, 0);
			}

			if(!e){
				e = entity_fn(Action::Create, eid, etype);
			}

			for(FieldBase* f = e->getFirstField(); f; f = f->getNextField()){
				f->readFromPacket(data);
				f->setUpdated(true);
			}

			e->markUpdated(true);
			entity_fn(Action::Update, eid, etype);
		}
	}

	if(section_bits & SNAPFLAG_UPD_SECTION){
		ecount.decode(data);
		for(size_t i = 0; i < ecount; ++i){
			uint16_t eid = UVarint().quickDecode(data);
			Entity* e = entity_fn(Action::Get, eid, 0);

			if(!e) return false;

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

			e->markUpdated(true);
			entity_fn(Action::Update, eid, e->getType());
		}
	}

	return true;
}

