#ifndef EXAMPLE_ENTITY_H
#define EXAMPLE_ENTITY_H
#include "nrg.h"
#include <cmath>
#include "constants.h"
namespace c = constants;

enum MyEntities {
	PLAYER,
	BALL
};

class EntityBase {
public:
	EntityBase(nrg::Entity* e) : xpos(e), ypos(e) {}
	short getX() const { return xpos.get(); }
	short getY() const { return ypos.get(); }
	short getXI() { return xpos.getInterp(); }
	short getYI() { return ypos.getInterp(); }
	void setX(int x){ xpos = x; }
	void setY(int y){ ypos = y; }
	uint16_t ID() const { return reinterpret_cast<const nrg::Entity*>(this)->getID(); }
protected:
	nrg::Field<short> xpos, ypos;
};

class PlayerEntity : public nrg::EntityHelper<PlayerEntity, PLAYER>, public EntityBase {
public:	
	PlayerEntity(int x) : EntityBase(this), score(this, 0){
		xpos = x;
		ypos = (c::screen_h - c::paddle_h) / 2;
	}
	void incScore(){
		score = score.get() + 1;
	}
	uint16_t getScore(){ return score.get(); }
private:
	nrg::Field<uint16_t> score;
};

class BallEntity : public nrg::EntityHelper<BallEntity, BALL>, public EntityBase {
public:
	BallEntity() : EntityBase(this), xv(-1.0f), yv(1.0f), speed(c::ball_speed) {
		reset();
	}
	void reset(){
		xpos = (c::screen_w - c::ball_size) / 2;
		ypos = (c::screen_h - c::ball_size) / 2;
		speed = c::ball_speed;
		xv *= -1.0f;
	}
	void update(){
		xpos = xpos.get() + xv * speed;
		ypos = ypos.get() + yv * speed;
	}
	float xv, yv, speed;
};

#endif
