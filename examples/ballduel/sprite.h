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
