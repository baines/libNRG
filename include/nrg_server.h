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
#include <map>
#include <set>
#include <vector>

namespace nrg {

class NRG_LIB Server {
public:
	Server(Input& input = null_input);
	Server(const NetAddress& bind_addr, Input& input = null_input);
	bool bind(const NetAddress& addr);
	bool isBound();
	size_t playerCount() const;
	bool update();
	bool pollEvent(Event& e);
	~Server();

	void registerEntity(Entity* e);
	void unregisterEntity(Entity* e);
	void markEntityUpdated(Entity* e);
	Player* getPlayerByID(uint16_t) const;
	
	const UDPSocket& getSocket() const { return sock; }
	const Snapshot& getSnapshot() const { return master_snapshot; }
	const DeltaSnapshotBuffer& getDeltaSnapshots() const { return snaps; }
	Input& getInput() const { return input; }
protected:
	void clearEntityUpdated(Entity* e);
	UDPSocket sock;
	Packet buffer;
	Input& input;
	EventQueue eventq;
	typedef std::map<NetAddress, Player*> ClientMap;
	ClientMap clients;
	Snapshot master_snapshot;
	DeltaSnapshotBuffer snaps;
	std::vector<Entity*> entities;
	std::set<uint16_t> updated_entities;
	IDAssigner<uint16_t> player_ids, entity_ids;
	uint64_t timer;
	int interval;
};

}

#endif
