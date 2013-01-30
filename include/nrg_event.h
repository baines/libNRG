#ifndef NRG_EVENT_H
#define NRG_EVENT_H
#include "nrg_core.h"
#include <queue>

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

struct NRG_LIB DisconnectEvent {
	uint8_t type; /* DISCONNECTED */
	const char* reason;
};

struct NRG_LIB EntityEvent {
	uint8_t type; /* ENTITY_{UPDATED, CREATED, DESTROYED} */
	uint16_t eid;
	uint16_t etype;
	Entity* pointer;
};

class NRG_LIB Player;

struct NRG_LIB PlayerEvent {
	uint8_t type; /* PLAYER_{JOIN, LEAVE} */
	uint16_t id;
	Player* player;
};

union NRG_LIB Event {
	uint8_t type;
	DisconnectEvent dc;
	EntityEvent entity;
	PlayerEvent join;
	
	Event() : type(0){}
	Event(const DisconnectEvent& e) : dc(e){}
	Event(const EntityEvent& e) : entity(e){}
	Event(const PlayerEvent& e) : join(e){}
};

class EventQueue {
public:
	EventQueue() : queue(){}
	void pushEvent(const Event& e){
		queue.push(e);
	}
	bool pollEvent(Event& e){
		if(queue.empty()){
			return false;
		} else {
			e = queue.front();
			queue.pop();
			return true;
		}
	}
	void clear(){
		while(!queue.empty()) queue.pop();
	}
private:
	std::queue<Event> queue;
};

};

#endif
