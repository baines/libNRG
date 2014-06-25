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

class Sprite {
public:
	Sprite(sf::Image i, ExampleEntity* e) : s(i), entity(e){ 
		update(true);
	}
	void update(bool interp){
		s.SetPosition(entity->getX(interp), entity->getY(interp));
	}
	void draw(sf::RenderWindow& win){
		win.Draw(s);
	}
	bool operator==(nrg::Entity* e){
		if(e->getType() != entity->getType()){
			return false;
		} else {
			return entity == static_cast<ExampleEntity*>(e);
		}
	}
private:
	sf::Sprite s;
	ExampleEntity* entity;
};


#endif
