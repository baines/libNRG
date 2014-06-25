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
#ifndef NRG_EVENT_H
#define NRG_EVENT_H
#include "nrg_core.h"
#include "nrg_queue.h"

namespace nrg {

typedef enum {
	/* Client-side */
	DISCONNECTED = 1,
	ENTITY_UPDATED,
	ENTITY_CREATED,
	ENTITY_DESTROYED,
	
	/* Server-side */
	PLAYER_JOIN,
	PLAYER_LEAVE,
	PLAYER_INPUT,
} EventType;

struct  DisconnectEvent {
	uint8_t type; /* DISCONNECTED */
	const char* reason;
};

class Entity;

struct  EntityEvent {
	uint8_t type; /* ENTITY_{UPDATED, CREATED, DESTROYED} */
	uint16_t eid;
	uint16_t etype;
	Entity* pointer;
};

class Player;

struct  PlayerEvent {
	uint8_t type; /* PLAYER_{JOIN, LEAVE} */
	uint16_t id;
	Player* player;
};

union  Event {
	uint8_t type;
	DisconnectEvent dc;
	EntityEvent entity;
	PlayerEvent player;
	
	Event() : type(0){}
	Event(const DisconnectEvent& e) : dc(e){}
	Event(const EntityEvent& e) : entity(e){}
	Event(const PlayerEvent& e) : player(e){}
};

class EventQueue {
public:
	EventQueue() : queue(32){}
	void pushEvent(const Event& e){
		queue.push(e);
	}
	bool pollEvent(Event& e){
		if(queue.empty()){
			return false;
		} else {
			e = queue.pop();
			return true;
		}
	}
	void clear(){
		queue.clear();
	}
private:
	Queue<Event> queue;
};

}

#endif
