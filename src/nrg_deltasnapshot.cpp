#include "nrg_snapshot.h"
#include "nrg_bit_io.h"
#include <cassert>

using namespace nrg;
using namespace std;

DeltaSnapshot::DeltaSnapshot() 
: id(-1) {
	entities.reserve(16);
	metadata.reserve(16);
	scratch.reserve(16);
}

void DeltaSnapshot::addEntity(Entity* e){
	assert(e && find(entities.begin(), entities.end(), e->getID()) == entities.end());
	entities.push_back({e->getNumFields(), e->getID(), e->getType()});
		
	uint16_t i = 0;
	for(FieldBase* f = e->getFirstField(); f; f = f->getNextField(), ++i){
		if(f->wasUpdated()){
			size_t off = field_data.tell(), sz = f->writeToPacket(field_data);
			FieldInfo fi = {e->getID(), i, sz, off};
			metadata.push_back(fi);
		}
	}	
}

void DeltaSnapshot::removeEntityById(uint16_t){

}

void DeltaSnapshot::reset(){
	entities.clear();
	metadata.clear();
	field_data.reset();
	id = -1;
}

void DeltaSnapshot::writeToPacket(Packet& p) const {
	size_t total_bytes = 0;
	for(auto& f : metadata)	total_bytes += f.size;
	for(auto& e : entities)	total_bytes += 5 + (e.num_fields-1)/8;
	p.write32(total_bytes);

	typedef vector<FieldInfo>::const_iterator F_cit;

	for(auto& e : entities){
		p.write16(e.id);
		p.write16(e.type);
	
		F_cit i = lower_bound(metadata.begin(), metadata.end(), e.id), j = i;
		
		BitWriter(p).writeFunc(e.num_fields, [&](int x){
			bool b = i != metadata.end() && *i == e.id && i->number == x;
			if(b) ++i;
			return b;
		});
		
		while(j != metadata.end() && *j == e.id){
			p.writeArray(j->get(field_data), j->size);
			++j;
		}
	}
}

// http://stackoverflow.com/questions/3633092/c-stl-sorted-vector-inplace-union
template<class T>
static void inplace_union(vector<T>& a, const vector<T>& b) {
    size_t mid = a.size();

    copy(b.begin(), b.end(), back_inserter(a));
    inplace_merge(a.begin(), a.begin() + mid, a.end());
    a.erase(unique(a.begin(), a.end()), a.end());
}

void DeltaSnapshot::mergeWithNext(const DeltaSnapshot& other){
	auto write_fn = [&](vector<FieldInfo>::const_iterator& i, const Packet& p){
		scratch.push_back(*i);
		scratch.back().offset = buffer.tell();
		buffer.writeArray(i->get(p), i->size);
		++i;
	};
	
	id = other.id;
	const DeltaSnapshot* const that = &other;
	size_t mid = entities.size();
	
	scratch.clear();
	buffer.reset();
	// make sure iterators are not invalidated in the set difference.
	entities.reserve(this->entities.size() + that->entities.size()); 
	
	set_symmetric_difference(entities.begin(), entities.begin()+mid,
		that->entities.begin(), that->entities.end(), back_inserter(entities)
	);
	
	for(size_t i = mid; i < entities.size(); ++i){
		uint16_t eid = entities[i].id;
		vector<FieldInfo>::const_iterator it;
		const DeltaSnapshot* s = this;
		
		if((it = find(metadata.begin(), metadata.end(), eid)) == metadata.end()){
			it = find(that->metadata.begin(), that->metadata.end(), eid);
			s = that;
		}
		
		while(it != s->metadata.end() && *it == eid) write_fn(it, s->field_data);
	}
	
	entities.resize(mid);
	
	set_intersection(entities.begin(), entities.begin()+mid, 
		that->entities.begin(), that->entities.end(), back_inserter(entities)
	);
	
	for(size_t i = mid; i < entities.size(); ++i){
		uint16_t eid = entities[i].id;
		vector<FieldInfo>::const_iterator old_it, new_it;
	
		old_it = lower_bound(this->metadata.begin(), this->metadata.end(), eid);
		new_it = lower_bound(that->metadata.begin(), that->metadata.end(), eid);
	
		bool ob, nb;
		while(ob = (old_it != this->metadata.end() && *old_it == eid), 
		      nb = (new_it != that->metadata.end() && *new_it == eid), ob || nb){
	
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
	metadata = scratch;
	field_data = buffer;
}
