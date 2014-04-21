#ifndef SPRITE_H
#define SPRITE_H
#include <SFML/Graphics.hpp>
#include "nrg.h"
#include "entities.h"

class Sprite {
public:
	Sprite(BallEntity* e)
	: interp(true)
	, s(c::ball)
	, entity(e)
	, type(BALL) {

	}

	Sprite(PlayerEntity* e)
	: interp(true)
	, s(c::paddle)
	, entity(e)
	, type(PLAYER) {

	}

	void draw(sf::RenderWindow& win){
		if(type == BALL){
			if(entity->getX() != (c::screen_w - c::ball_size) / 2) interp = true;
			if(abs(entity->getX() - s.GetPosition().x) > c::screen_w / 4) interp = false;
		}
	
		if(interp){
			s.SetPosition(entity->getXI(), entity->getYI());
		} else {
			s.SetPosition(entity->getX(), entity->getY());
		}
		win.Draw(s);
	}

	bool operator==(uint16_t id){
		return id == entity->ID();
	}
private:
	bool interp;
	sf::Shape s;
	EntityBase* entity;
	uint16_t type;
};

#endif
