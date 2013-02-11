#ifndef EXAMPLE_ENTITY_H
#define EXAMPLE_ENTITY_H
#include "nrg.h"
#include <cmath>

enum MyEntities {
	PLAYER,
	BALL
};

class EntityBase {
public:
	EntityBase(nrg::Entity* e) : xpos(e), ypos(e), e(e) {}
	uint16_t getX() const { return xpos.get(); }
	uint16_t getY() const { return ypos.get(); }
	uint16_t getID() const { return e->getID(); }
protected:
	nrg::Field<uint16_t> xpos, ypos;
	nrg::Entity* e;
};

class PlayerEntity : public EntityBase, public nrg::EntityHelper<PlayerEntity, PLAYER> {
public:	
	PlayerEntity(int x) : EntityBase(this), score(this, 0){
		xpos = x;
		ypos = 240;
	}
	void getFields(nrg::FieldList& fl){
		fl.add(xpos).add(ypos).add(score);
	}
	void setY(int y){
		ypos = y;
	}
private:
	nrg::Field<uint16_t> score;
};

class BallEntity : public EntityBase, public nrg::EntityHelper<BallEntity, BALL> {
public:
	BallEntity() : EntityBase(this), xv(1.0f), yv(1.0f) {
		xpos = 320;
		ypos = 240;
	}
	void getFields(nrg::FieldList& fl){
		fl.add(xpos).add(ypos);
	}
	void update(){
		xpos = xpos.get() + xv;
		ypos = ypos.get() + yv;
	}		
private:
	float xv, yv;
};

#endif
