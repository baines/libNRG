#ifndef NRG_INPUT_H
#define NRG_INPUT_H
#include "nrg_core.h"
#include "nrg_field.h"
#include "nrg_player.h"

namespace nrg {

struct NRG_LIB Input : public FieldContainer {
	virtual void getFields(FieldList& list) = 0;
	virtual void onUpdateNRG(Player& player){}

	void markUpdated();
	bool readFromPacket(Packet& p);
	void writeToPacket(Packet& p);
};

struct NRG_LIB NullInput : public Input {
	void getFields(FieldList& fl){}
};

extern NullInput null_input;

}

#endif
