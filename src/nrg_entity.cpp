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
#include "nrg_entity.h"
#include "nrg_server.h"
#include "nrg_client_state.h"

using namespace nrg;

Entity::Entity()
: nrg_id(0)
, nrg_updated(false)
, manager(nullptr) {

}

void Entity::markUpdated(bool b){
	if(!nrg_updated && b && manager){
		manager->markEntityUpdated(*this);
	}
	nrg_updated = b;
}

Entity::~Entity(){
	if(manager)	manager->unregisterEntity(*this);
}

double Entity::getInterpTimer() const {
	return manager ? manager->getInterpTimer() : 1.0;
}


