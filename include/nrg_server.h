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
#include <unordered_set>
#include <vector>

namespace nrg {

class NRG_LIB Server : public EntityManager {
public:
	Server(const char* game_name, uint32_t game_version, InputBase& input);
	Server(const char* game_name, uint32_t game_version);
		
	bool bind(const NetAddress& addr);
	bool isBound();
	
	bool update();
	bool pollEvent(Event& e);
	
	size_t playerCount() const;
	
	void setTickRate(uint8_t rate);
	uint8_t getTickRate() const;
	
	void registerEntity(Entity& e);
	void unregisterEntity(Entity& e);
	void markEntityUpdated(Entity& e);
	
	template<class M, class F>
	void addMessageHandler(const F& f){
		messages.insert(make_unique<M>(f));
	}
	void broadcastMessage(const MessageBase& m);
	
	Player* getPlayerByID(uint16_t) const;
	const UDPSocket& getSocket() const { return sock; }
	const Snapshot& getSnapshot() const { return master_snapshot; }
	const DeltaSnapshotBuffer& getDeltaSnapshots() const { return snaps; }
	InputBase* getInput() const { return input; }
	
	virtual ~Server();
protected:
	UDPSocket sock;
	Packet buffer;
	InputBase* input;
	EventQueue eventq;
	std::map<NetAddress, Player*> clients;
	Snapshot master_snapshot;
	DeltaSnapshotBuffer snaps;
	std::vector<Entity*> entities;
	std::vector<uint16_t> updated_entities;
	std::map<uint16_t, std::unique_ptr<MessageBase>> messages;
	IDAssigner<uint16_t> player_ids, entity_ids;
	uint64_t timer;
	int interval;
};

}

#endif
