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

	void update(){
		angle += M_PI / 45.0f;

		xpos = 320 + (200 * sin(angle));
		ypos = 240 + (200 * cos(angle));
	}
	
	uint16_t getX(bool interp) const {
		if(interp) return xpos.getInterp();
		else return xpos.get();
	}

	uint16_t getY(bool interp) const {
		if(interp) return ypos.getInterp();
		else return ypos.get();
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
