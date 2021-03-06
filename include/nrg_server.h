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
 * Contains the main server-side class of the library
 */
#ifndef NRG_SERVER_H
#define NRG_SERVER_H
#include "nrg_core.h"
#include "nrg_version.h"
#include "nrg_socket.h"
#include "nrg_packet.h"
#include "nrg_netaddress.h"
#include "nrg_snapshot.h"
#include "nrg_event.h"
#include "nrg_player.h"
#include "nrg_input.h"
#include "nrg_message.h"
#include <map>
#include <set>
#include <vector>

namespace nrg {

/** The main server-side class of the library */
class Server : public EntityManager {
public:
	/** Construct a server with the given game name, version and a user-created subclass of Input */
	Server(const std::string& game_name, uint32_t game_version, InputBase& input);
	/** Construct a server which doesn't accept input, generally not very useful */
	Server(const std::string& game_name, uint32_t game_version);

	/** Bind the server to the specified local address */
	bool bind(const NetAddress& addr);

	/** Returns true if the server is bound to a port */
	bool isBound() const;

	/** Blocks to accept input from clients until the next snapshot time, then creates the new snapshot and sends updates to clients - returns false on error */
	bool update();

	/** Receives Event instances into \p e, returns false if there are no more events */
	bool pollEvent(Event& e);

	/** @internal Add an event to the server's internal event queue */
	void pushEvent(const Event& e);

	/** Returns the number of players connected to this server */
	size_t playerCount() const;

	/** Sets the maximum number of players, doesn't kick anyone if the current count is larger than this */
	void setMaxPlayers(uint16_t val) { max_players = val; }

	/** Returns true if the server has reached its maximum player count */
	bool isFull() const { return max_players == 0 ? false : playerCount() >= max_players; }

	/** Sets the number of ticks / master snapshots generated by the server per second, 256 max */
	void setTickRate(uint8_t rate);

	/** Gets the number of ticks per second the server is running at, default: 20 */
	uint8_t getTickRate() const;

	/** Register an Entity with the server to be tracked and replicated to clients */
	void registerEntity(Entity& e);

	/** @internal Unregister an Entity - automatically called by Entity's destructor */
	void unregisterEntity(Entity& e);

	/** @internal Lets the server know an entity has been updated, done automatically */
	void markEntityUpdated(Entity& e);

	/** Adds a callback function \p func that will be called on receipt of a message of type \p M */
	template<class M, class F>
	void addMessageHandler(F&& func){
		using namespace std;
		using namespace std::placeholders;

		global_msg_handlers.insert(
			make_unique<M>(std::bind(forward<F>(func), _1, _2, ref(current_player)))
		);
	}

	/** Sends message \p m to all connected clients */
	void broadcastMessage(const MessageBase& m);

	/** Gets the player with the given id, or \p nullptr if they don't exist */
	Player* getPlayerByID(uint16_t) const;

	/** @name Various Getters */
	/** @{ */
	const UDPSocket&           getSocket() const         { return sock; }
	const Snapshot&            getSnapshot() const       { return master_snapshot; }
	const DeltaSnapshotBuffer& getDeltaSnapshots() const { return snaps; }
	InputBase*                 getInput() const          { return input; }
	const std::string&         getGameName()             { return game_name; }
	const uint32_t             getGameVersion()          { return game_version; }
	/** @} */

	/** Sets a function that should return true if two uint32_t game version parameters are compatible */
	template<class F>
	void setVersionComparison(F&& func){
		version_func = std::forward<F>(func);
	}

	/** Runs its internal version comparison function against the server version and \p client_ver, returns true if they're compatible */
	bool isGameVersionCompatible(uint32_t client_ver) const;

	/** Associate a user-given pointer with this server */
	void  setUserPointer(void* p) { user_pointer = p; }

	/** Returns the user-given pointer associated with this server, nullptr by default */
	void* getUserPointer() const  { return user_pointer; }

	/** Default destructor */
	virtual ~Server();
private:
	UDPSocket sock;
	Packet buffer;
	InputBase* input;
	EventQueue eventq;
	std::map<NetAddress, Player*> clients;
	std::set<std::unique_ptr<MessageBase>> global_msg_handlers;
	Snapshot master_snapshot;
	DeltaSnapshotBuffer snaps;
	std::vector<Entity*> entities;
	std::vector<uint16_t> updated_entities;
	IDAssigner<uint16_t> player_ids, entity_ids;
	Player* current_player;
	uint64_t timer;
	std::string game_name;
	uint32_t game_version;
	uint16_t max_players;
	std::function<bool(uint32_t, uint32_t)> version_func;
	std::function<bool(Server& s, Packet& p)> connect_check;
	void* user_pointer;
	int interval;
};

}

#endif
