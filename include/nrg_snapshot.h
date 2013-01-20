#ifndef NRG_SNAPSHOT_H
#define NRG_SNAPSHOT_H
#include "nrg_core.h"
#include "nrg_entity.h"
#include "nrg_event.h"
#include <vector>
#include <map>

namespace nrg {

class NRG_LIB Snapshot {
public:
	Snapshot();
	Snapshot(uint16_t id);
	int getID() const { return id; }
	void setID(uint16_t id){ this->id = id; }
	void addEntity(Entity* e, FieldList& fl);
	virtual void removeEntityById(uint16_t id);
	bool mergeWithNext(const Snapshot& next);
	void writeToPacket(Packet& p) const;
	void reset();
protected:
	struct NRG_LIB EntityData {
		uint16_t id, type;
		std::vector<size_t> field_sizes;
		Packet field_data;
		off_t getFieldOffset(int num) const;
		size_t getTotalBytes() const;
	};
	int id;
	std::map<uint16_t, EntityData> edata;
	Packet buffer;
};

struct NRG_LIB DeltaSnapshot : public Snapshot {
	virtual void removeEntityById(uint16_t id);
};

class NRG_LIB ClientSnapshot {
public:
	ClientSnapshot() : data() {};
	bool readFromPacket(Packet& p);
	void applyUpdate(std::vector<Entity*>& entities, 
		const std::map<uint16_t, Entity*>& entity_types, EventQueue& eq);
	void reset();
private:
	Packet data;
};

class NRG_LIB DeltaSnapshotBuffer {
	static const int N = NRG_NUM_PAST_SNAPSHOTS;
public:
	DeltaSnapshotBuffer() : snaps(), cur_id(0){};
	DeltaSnapshot* find(uint16_t id) {
		DeltaSnapshot* s = &snaps[id % N];	
		return s->getID() == id ? s : NULL;
	}
	const DeltaSnapshot* find(uint16_t id) const {
		const DeltaSnapshot* s = &snaps[id % N];	
		return s->getID() == id ? s : NULL;
	}
	DeltaSnapshot& next(){
		DeltaSnapshot& ds = snaps[cur_id % N];
		ds.reset();
		ds.setID(cur_id);
		cur_id++;
		return ds;
	}
	uint16_t getCurrentID() const {
		return cur_id;
	}
private:
	DeltaSnapshot snaps[N];
	uint16_t cur_id;
};

};

#endif
