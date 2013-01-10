#ifndef EXAMPLE_ENTITY_H
#define EXAMPLE_ENTITY_H
#include "nrg.h"
#include <cmath>

enum MyEntities {
	EXAMPLE
};

class ExampleEntity : public nrg::EntityHelper<ExampleEntity, EXAMPLE> {
public:	
	ExampleEntity() : timer(0.0f), xpos(this, 320), ypos(this, 240){}

	virtual void getFields(nrg::FieldList& list){
		list.add(xpos).add(ypos);
	}

	void update(){
		timer += M_PI / 180.0f;

		xpos = 320 + (60 * sin(timer));
		ypos = 240 + (60 * cos(timer));
	}
	
	uint16_t getx() const {
		return xpos.get();
	}

	uint16_t gety() const {
		return ypos.get();
	}
private:
	float timer;
	nrg::Field<uint16_t> xpos, ypos;
};

#endif
