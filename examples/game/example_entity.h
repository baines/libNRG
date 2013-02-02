#ifndef EXAMPLE_ENTITY_H
#define EXAMPLE_ENTITY_H
#include "nrg.h"
#include <cmath>

enum MyEntities {
	EXAMPLE
};

class ExampleEntity : public nrg::EntityHelper<ExampleEntity, EXAMPLE> {
public:	
	ExampleEntity() : angle(0.0f), xpos(this, 320), ypos(this, 240), array(this){}

	virtual void getFields(nrg::FieldList& list){
		list.add(xpos).add(ypos).add(array);
	}

	void update(){
		angle += M_PI / 90.0f;

		xpos = 320 + (200 * sin(angle));
		ypos = 240 + (200 * cos(angle));
	}
	
	uint16_t getX() const {
		return xpos.get();
	}

	uint16_t getY() const {
		return ypos.get();
	}
	
	uint8_t getArray(int i){
		return array.get(i);
	}
private:
	float angle;
	nrg::Field<uint16_t> xpos, ypos;
	nrg::Field<uint8_t[8]> array;
};

#endif
