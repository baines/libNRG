#ifndef NRG_SNAPSHOT_H
#define NRG_SNAPSHOT_H
#include "nrg_core.h"
#include "nrg_entity.h"
#include <vector>
#include <set>

namespace nrg {

class NRG_LIB Snapshot {
public:
	Snapshot();
	Snapshot(uint16_t id);
	Snapshot(const Snapshot& copy);
	Snapshot& operator=(const Snapshot& other);

	uint16_t getID() const;
	void setID(uint16_t id);
	void addEntity(const Entity* e);
	bool merge(const Snapshot& other);
	void resetAndIncrement();
protected:
	struct NRG_LIB EntityInfo {
		uint16_t id;
		off_t start;
		std::vector<size_t> field_sizes;
		bool operator<(const EntityInfo& other) const {
			return id < other.id;
		}
	};
	int id;
	std::set<EntityInfo> stored_entities;
	Packet field_data;
};

};

#endif
