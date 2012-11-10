#ifndef NRG_INPUT_H
#define NRG_INPUT_H

namespace nrg {

class NRG_LIB Input {
	Input();
	void beginFrame();
	void endFrame();
	void addEvent(int key, int value);

	bool acknowledge(uint16_t id);
	void toPacket(Packet& p);
protected:
	uint16_t input_id;

};

};

#endif
