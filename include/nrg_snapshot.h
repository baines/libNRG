#ifndef NRG_SNAPSHOT_H
#define NRG_SNAPSHOT_H
#include "nrg_core.h"
#include "nrg_entity.h"

namespace nrg {

class NRG_LIB Snapshot {
public:
	Snapshot();
	Snapshot(uint16_t id);
	uint16_t getID() const;
	void setID(uint16_t id);

	void addEntity(const Entity* e);
	void merge(const Snapshot& other);
	void resetAndIncrement();
protected:
	int id;
	Packet data;
};

};

#endif
