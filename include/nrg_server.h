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
#ifndef NRG_SERVER_H
#define NRG_SERVER_H
#include "nrg_core.h"
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

class Server : public EntityManager {
public:
	Server(const std::string& game_name, uint32_t game_version, InputBase& input);
	Server(const std::string& game_name, uint32_t game_version);

	bool bind(const NetAddress& addr);
	bool isBound() const;

	bool update();
	bool pollEvent(Event& e);
	void pushEvent(const Event& e);
	
	size_t playerCount() const;
	void setMaxPlayers(uint16_t val) { max_players = val; }
	bool isFull() const { return max_players == 0 ? false : playerCount() >= max_players; }

	void setTickRate(uint8_t rate);
	uint8_t getTickRate() const;

	void registerEntity(Entity& e);
	void unregisterEntity(Entity& e);
	void markEntityUpdated(Entity& e);

	template<class M, class F>
	void addMessageHandler(F&& func){
		using namespace std;
		using namespace std::placeholders;
		
		global_msg_handlers.insert(
			make_unique<M>(std::bind(forward<F>(func), _1, _2, ref(current_player)))
		);
	}
	void broadcastMessage(const MessageBase& m);

	Player* getPlayerByID(uint16_t) const;
	
	const UDPSocket&           getSocket() const         { return sock; }
	const Snapshot&            getSnapshot() const       { return master_snapshot; }
	const DeltaSnapshotBuffer& getDeltaSnapshots() const { return snaps; }
	InputBase*                 getInput() const          { return input; }
	const std::string&         getGameName()             { return game_name; }
	const uint32_t             getGameVersion()          { return game_version; }
	
	template<class F>
	void setVersionComparison(F&& func){
		version_func = std::forward<F>(func);
	}
	
	bool isGameVersionCompatible(uint32_t client_ver) const;

	void  setUserPointer(void* p) { user_pointer = p; }
	void* getUserPointer() const  { return user_pointer; }

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
