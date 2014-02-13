#ifndef NRG_SNAPSHOT_H
#define NRG_SNAPSHOT_H
#include "nrg_core.h"
#include "nrg_entity.h"
#include "nrg_event.h"
#include "nrg_ringbuffer.h"
#include <vector>
#include <map>

namespace nrg {

class NRG_LIB Snapshot {
public:
	Snapshot();
	Snapshot(uint16_t id);
	uint16_t getID() const { return id; }
	void setID(uint16_t id){ this->id = id; }
	void addEntity(Entity* e);
	void removeEntityById(uint16_t id);
	void writeToPacket(Packet& p) const;
	void reset();
protected:
	struct NRG_LIB EntityData {
		uint16_t eid, etype;
		std::vector<size_t> field_sizes;
		Packet field_data;
		off_t getFieldOffset(int num) const;
		size_t getTotalBytes() const;
	};
	uint16_t id;
	std::map<uint16_t, EntityData> edata;
	Packet buffer;
};

struct NRG_LIB DeltaSnapshot {
	DeltaSnapshot() : DeltaSnapshot(0){}
	DeltaSnapshot(int i);
	uint16_t getID() const { return id; }
	void setID(uint16_t id){ this->id = id; }
	void addEntity(Entity* e);
	void removeEntityById(uint16_t id);
	void mergeWithNext(const DeltaSnapshot& next);
	void writeToPacket(Packet& p) const;
	void reset();
private:
	struct FieldInfo {
		uint16_t entity;
		uint16_t number;
		size_t size;
		size_t offset;
		const uint8_t* get(const Packet& p) const {
			return p.getBasePointer() + offset;
		}
		bool operator<(uint16_t v) const { return entity < v; }
		bool operator==(uint16_t v) const { return entity == v; }
	};
	struct EntityInfo {
		size_t num_fields;
		uint16_t id;
		uint16_t type;
		bool full;
		bool operator<(const EntityInfo& other) const { return id < other.id; }
		bool operator==(const EntityInfo& other) const { return id == other.id;	}
		bool operator==(uint16_t v) const {	return id == v;	}
	};
	
	uint16_t id;
	std::vector<EntityInfo> entities;
	std::vector<FieldInfo> fields, tmp_fields;
	size_t full_count, del_count, upd_count;
	Packet field_data, buffer;
};

class NRG_LIB ClientSnapshot {
	typedef std::vector<Entity*> EVec;
	typedef std::map<uint16_t, Entity*> EMap;
public:
	ClientSnapshot() : data() {}
	bool readFromPacket(Packet& p);
	void applyUpdate(EVec& entities, const EMap& entity_types, EventQueue& eq);
	void reset();
private:
	Packet data;
};

typedef RingBuffer<DeltaSnapshot, NRG_NUM_PAST_SNAPSHOTS> DeltaSnapshotBuffer;

}

#endif
