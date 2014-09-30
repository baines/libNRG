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
#include <cassert>

using namespace nrg;
using namespace std;

namespace {

// http://stackoverflow.com/questions/3633092/c-stl-sorted-vector-inplace-union
template<class T>
static void inplace_union(vector<T>& a, const vector<T>& b) {
    size_t mid = a.size();

    copy(b.begin(), b.end(), back_inserter(a));
    inplace_merge(a.begin(), a.begin() + mid, a.end());
    /* Use reverse iterators here, since unique() keeps only the first item it
       finds, and the new EntityInfo structs will appear after the old ones. */
    a.erase(move(unique(a.rbegin(), a.rend()).base(), a.end(), a.begin()), a.end());
}

enum {
	SNAPFLAG_DEL_SECTION  = 0x80,
	SNAPFLAG_FULL_SECTION = 0x40,
	SNAPFLAG_UPD_SECTION  = 0x20
};

}

DeltaSnapshot::DeltaSnapshot(int i)
: id(i)
, full_count(0)
, del_count(0)
, upd_count(0) {
	entities.reserve(16);
	fields.reserve(16);
	tmp_fields.reserve(16);
}

// XXX: Must be added in sorted order or this will break
void DeltaSnapshot::addEntity(Entity* e){
	assert(e && find(entities.begin(), entities.end(), e->getID()) == entities.end());
	entities.push_back({e->getNumFields(), e->getID(), e->getType()});

	uint16_t i = 0, upd = 0;
	for(FieldBase* f = e->getFirstField(); f; f = f->getNextField(), ++i){
		if(f->wasUpdated()){
			++upd;
			size_t off = field_data.tell(), sz = f->writeToPacket(field_data);
			FieldInfo fi = {e->getID(), i, sz, off};
			fields.push_back(fi);
		}
	}
	if((entities.back().full = (upd == e->getNumFields()))){
		++full_count;
	} else {
		++upd_count;
	}
}

void DeltaSnapshot::removeEntityByID(uint16_t eid){
	assert(find(entities.begin(), entities.end(), eid) == entities.end());
	entities.push_back({0, eid, 0, false});
	++del_count;
}

void DeltaSnapshot::reset(){
	entities.clear();
	fields.clear();
	field_data.reset();
	id = 0;
}

void DeltaSnapshot::writeToPacket(Packet& p) const {
	uint8_t section_bits = 0;

	if(del_count)  section_bits |= SNAPFLAG_DEL_SECTION;
	if(full_count) section_bits |= SNAPFLAG_FULL_SECTION;
	if(upd_count)  section_bits |= SNAPFLAG_UPD_SECTION;

	p.write8(section_bits);

	if(del_count){
		UVarint(del_count).encode(p);
		for(auto& e : entities){
			if(e.num_fields == 0){
				UVarint(e.id).encode(p);
			}
		}
	}

	if(full_count){
		UVarint(full_count).encode(p);
		for(auto& e : entities){
			if(!e.full) continue;

			UVarint(e.id).encode(p);
			// TODO: optimisation: don't send entity type if client already knows it.
			UVarint(e.type).encode(p);

			auto i = lower_bound(fields.begin(), fields.end(), e.id);
			while(i != fields.end() && *i == e.id){
				p.writeArray(i->get(field_data), i->size);
				++i;
			}
		}
	}

	if(upd_count){
		UVarint(upd_count).encode(p);
		for(auto& e : entities){
			if(e.full || e.num_fields == 0) continue;

			UVarint(e.id).encode(p);

			auto i = lower_bound(fields.begin(), fields.end(), e.id), j = i;

			BitWriter(p).writeFunc(e.num_fields, [&](int x){
				bool b = i != fields.end() && *i == e.id && i->number == x;
				if(b) ++i;
				return b;
			});

			while(j != fields.end() && *j == e.id){
				p.writeArray(j->get(field_data), j->size);
				++j;
			}
		}
	}
}

void DeltaSnapshot::mergeWithNext(const DeltaSnapshot& other){
	auto write_fn = [&](vector<FieldInfo>::const_iterator& i, const Packet& p){
		tmp_fields.push_back(*i);
		tmp_fields.back().offset = buffer.tell();
		buffer.writeArray(i->get(p), i->size);
		++i;
	};

	id = other.id;
	const DeltaSnapshot* const that = &other;
	size_t mid = entities.size();

	tmp_fields.clear();
	buffer.reset();

	// make sure iterators are not invalidated in the set difference.
	entities.reserve(this->entities.size() + that->entities.size());

	set_symmetric_difference(that->entities.begin(), that->entities.end(),
		entities.begin(), entities.begin()+mid, back_inserter(entities)
	);

	for(size_t i = mid; i < entities.size(); ++i){
		uint16_t eid = entities[i].id;
		vector<FieldInfo>::const_iterator it;
		const DeltaSnapshot* s = this;

		if((it = find(fields.begin(), fields.end(), eid)) == fields.end()){
			it = find(that->fields.begin(), that->fields.end(), eid);
			s = that;
		}

		while(it != s->fields.end() && *it == eid) write_fn(it, s->field_data);
	}

	entities.resize(mid);

	set_intersection(that->entities.begin(), that->entities.end(),
		entities.begin(), entities.begin()+mid, back_inserter(entities)
	);

	for(size_t i = mid; i < entities.size(); ++i){
		uint16_t eid = entities[i].id;

		// If this entity was deleted in the newer snapshot, don't add the old fields.
		if(entities[i].num_fields == 0) continue;

		vector<FieldInfo>::const_iterator old_it, new_it;
		old_it = lower_bound(this->fields.begin(), this->fields.end(), eid);
		new_it = lower_bound(that->fields.begin(), that->fields.end(), eid);

		bool ob = false, nb = false;
		while(ob = (old_it != this->fields.end() && *old_it == eid),
		      nb = (new_it != that->fields.end() && *new_it == eid), ob || nb){

			if(!ob || (nb && new_it->number <= old_it->number)) {
				if(new_it->number == old_it->number) ++old_it;
				write_fn(new_it, that->field_data);
			} else {
				write_fn(old_it, this->field_data);
			}
		}
	}

	entities.resize(mid);

	inplace_union(this->entities, that->entities);
	fields = tmp_fields;
	field_data = buffer;

	full_count = del_count = upd_count = 0;

	for(auto& e : entities){
		if(e.num_fields == 0){
			++del_count;
		} else if(e.full){
			++full_count;
		} else {
			++upd_count;
		}
	}
}
