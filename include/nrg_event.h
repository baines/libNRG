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
/** @file
 *  Functionality to notify users of Client and Server occurances
 */
#ifndef NRG_EVENT_H
#define NRG_EVENT_H
#include "nrg_core.h"
#include "nrg_queue.h"

namespace nrg {

/** Event types */
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

/** Event raised when the Client becomes disconnected */
struct DisconnectEvent {
	uint8_t type;       /**< Will be DISCONNECTED */
	const char* reason; /**< Statically-allocated reason for the disconnection */
};

class Entity;

/** Event raised when entities are updated, created, destroyed client-side - an alternative to the virtual methods that Entity provides */
struct EntityEvent {
	uint8_t type;    /**< Will be ENTITY_{UPDATED, CREATED, DESTROYED} */
	uint16_t eid;    /**< The Entity's ID */
	uint16_t etype;  /**< The Entity's user-defined type id */
	Entity* pointer; /**< Pointer to the Entity - don't dereference it on ENTITY_DESTROYED! */
};

class Player;

/** Event raised on the server when a player joins or leaves */
struct PlayerEvent {
	uint8_t type;   /**< Will be PLAYER_{JOIN, LEAVE} */
	uint16_t id;    /**< The player's ID assigned by the library */
	Player* player; /**< Pointer to the Player - don't dereference it on PLAYER_LEAVE! */
};

/** Union to contain all the event types */
union Event {
	uint8_t type;       /**< Used to determine which event this is */
	DisconnectEvent dc; /**< For DISCONNECTED */
	EntityEvent entity; /**< For ENTITY_{UPDATED, CREATED, DESTROYED} */
	PlayerEvent player; /**< For PLAYER_{JOIN, LEAVE} */

	/** Default Constructor */
	Event() : type(0){}

	/** Implicit conversion constructor from DisconnectEvent */
	Event(const DisconnectEvent& e) : dc(e){}

	/** Implicit conversion constructor from EntityEvent */
	Event(const EntityEvent& e) : entity(e){}

	/** Implicit conversion constructor from PlayerEvent */
	Event(const PlayerEvent& e) : player(e){}
};

/** Holds a queue of Event objects */
class EventQueue {
public:
	/** Default Constructor */
	EventQueue() : queue(32){}

	/** Add an event to the end of the queue */
	void pushEvent(const Event& e){
		queue.push(e);
	}

	/** Place the Event at the head of the queue into \p e - return true if this happened or false if the queue is empty */
	bool pollEvent(Event& e){
		if(queue.empty()){
			return false;
		} else {
			e = queue.pop();
			return true;
		}
	}

	/** Removes all Events from the queue */
	void clear(){
		queue.clear();
	}
private:
	Queue<Event> queue;
};

}

#endif
