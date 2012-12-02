#ifndef NRG_INPUT_H
#define NRG_INPUT_H
#include "nrg_core.h"
#include "nrg_packet.h"
#include <map>

namespace nrg {

class NRG_LIB Input {
	Input();
	Input& nextFrame();
	void addEvent(int32_t key, int32_t value);
	void acknowledge(uint32_t id);
	void writeToPacket(Packet& p);
	void readFromPacket(Packet& p);
protected:
	uint32_t input_id;
	struct Event {
		int32_t value;
		uint32_t state_added;
	};
	std::map<int32_t, Event> events;
};

};

#endif
