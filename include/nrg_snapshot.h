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
	Snapshot(const Snapshot& copy);
	Snapshot& operator=(const Snapshot& other);

	int getID() const { return id; }
	void setID(uint16_t id);
	void addEntity(Entity* e);
	void removeEntityById(uint16_t id);
	bool merge(const Snapshot& other);
	void reset();
	void resetAndIncrement();

	void writeToPacket(Packet& p);
	bool readFromPacket(Packet& p);
	void applyUpdate(std::vector<Entity*>& entities, 
		const std::map<uint16_t, Entity*>& entity_types, EventQueue& eq);
protected:
	struct NRG_LIB EntityInfo {
		uint16_t id, type;
		off_t start;
		std::vector<size_t> field_sizes;
		off_t getFieldOffset(int num) const;
	};
	int id;
	std::map<uint16_t, EntityInfo> edata;
	Packet field_data;
};

};

#endif
