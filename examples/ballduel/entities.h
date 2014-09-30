/*
  LibNRG - Networking for Real-time Games

  Copyright (C) 2012-2014 Alex Baines <alex@abaines.me.uk>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#ifndef ENTITIES_H
#define ENTITIES_H
#include <algorithm>
#include "nrg.h"
#include <cmath>
#include "constants.h"
namespace c = constants;

#ifdef CLIENTSIDE
#include "client.h"
#endif

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
#ifdef CLIENTSIDE
	void onCreate(nrg::Client& c){ clientAddEntity(this); }
	void onUpdate(nrg::Client& c){ clientSetScore((getX() < c::screen_w / 2), score.get()); }
	void onDestroy(nrg::Client& c){ clientDelEntity(getID()); }
#endif
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
#ifdef CLIENTSIDE
	void onCreate(nrg::Client& c){ clientAddEntity(this); }
	void onDestroy(nrg::Client& c){ clientDelEntity(getID()); }
#endif
	float xv, yv, speed;
};

#endif
