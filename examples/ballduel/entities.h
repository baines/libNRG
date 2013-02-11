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
	short getX() const { return xpos.get(); }
	short getY() const { return ypos.get(); }
	short getXI() const { return xpos.getInterp(); }
	short getYI() const { return ypos.getInterp(); }
	void setX(int x){ xpos = x; }
	void setY(int y){ ypos = y; }
	uint16_t getID() const { return e->getID(); }
protected:
	nrg::Field<short> xpos, ypos;
	nrg::Entity* e;
};

class PlayerEntity : public nrg::EntityHelper<PlayerEntity, PLAYER>, public EntityBase {
public:	
	PlayerEntity(int x) : EntityBase(this), score(this, 0){
		xpos = x;
		ypos = 240;
	}
	void incScore(){
		score = score.get()+1;
	}
	uint16_t getScore(){ return score.get(); }
private:
	nrg::Field<uint16_t> score;
};

class BallEntity : public nrg::EntityHelper<BallEntity, BALL>, public EntityBase {
public:
	BallEntity() : EntityBase(this), xv(-10.0f), yv(10.0f), speed(2.0f) {
		reset();
	}
	void reset(){
		xpos = 320;
		ypos = 240;
		speed = 1.0f;
		xv *= -1.0f;
	}
	void update(){
		xpos = xpos.get() + xv * speed;
		ypos = ypos.get() + yv * speed;
	}
	float xv, yv, speed;
};

#endif
