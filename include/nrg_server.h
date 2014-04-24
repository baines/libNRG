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
	void addMessageHandler(F&& func){
		using namespace std;
		using namespace std::placeholders;
		
		global_msg_handlers.insert(
			make_unique<M>(std::bind(forward<F>(func), _1, _2, ref(current_player)))
		);
	}
	void broadcastMessage(const MessageBase& m);

	Player* getPlayerByID(uint16_t) const;
	const UDPSocket& getSocket() const { return sock; }
	const Snapshot& getSnapshot() const { return master_snapshot; }
	const DeltaSnapshotBuffer& getDeltaSnapshots() const { return snaps; }
	InputBase* getInput() const { return input; }

	void setUserPointer(void* p){ user_pointer = p; }
	void* getUserPointer() const { return user_pointer; }

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
	void* user_pointer;
	int interval;
};

}

#endif
