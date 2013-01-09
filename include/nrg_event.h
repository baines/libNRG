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

class NRG_LIB PlayerConnection;

struct NRG_LIB PlayerJoinEvent {
	uint8_t type; /* PLAYER_JOIN */
	uint16_t id;
	PlayerConnection* player;
};

struct NRG_LIB PlayerLeaveEvent {
	uint8_t type; /* PLAYER_LEAVE */
	uint16_t id;
};

union NRG_LIB Event {
	uint8_t type;
	DisconnectEvent dc;
	EntityEvent entity;
	PlayerJoinEvent join;
	PlayerLeaveEvent leave;
	
	Event() : type(0){}
	Event(const DisconnectEvent& e) : dc(e){}
	Event(const EntityEvent& e) : entity(e){}
	Event(const PlayerJoinEvent& e) : join(e){}
	Event(const PlayerLeaveEvent& e) : leave(e){}
};

class EventQueue {
public:
	EventQueue() : queue(){}
	template<class E>
	void pushEvent(const E& e){
		queue.push(Event(e));
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
private:
	std::queue<Event> queue;
};

};

#endif
