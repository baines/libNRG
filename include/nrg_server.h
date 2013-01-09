#ifndef NRG_SERVER_H
#define NRG_SERVER_H
#include "nrg_core.h"
#include "nrg_socket.h"
#include "nrg_packet.h"
#include "nrg_netaddress.h"
#include "nrg_connection.h"
#include "nrg_snapshot.h"
#include "nrg_state.h"
#include "nrg_event.h"
#include <map>
#include <vector>

namespace nrg {

class NRG_LIB PlayerConnection;

class NRG_LIB Server {
public:
	Server();
	Server(const NetAddress& bind_addr);
	void bind(const NetAddress& addr);
	bool isBound();
	size_t playerCount() const;
	status_t update();
	~Server();

	void registerEntity(Entity* e);
	void markEntityUpdated(Entity* e);
protected:
	friend class PlayerConnection;
	UDPSocket sock;
	Packet buffer;
	EventQueue eventq;
	typedef std::map<NetAddress, PlayerConnection*> ClientMap;
	ClientMap clients;
	Snapshot master_snapshot;
	std::vector<Entity*> entities, updated_entities;
	uint64_t timer;
	int interval;
};

class NRG_LIB PlayerConnection {
public:
	PlayerConnection(uint16_t id, const Server& server, const NetAddress& addr);
	bool addPacket(Packet& p);
	bool update();
	uint16_t getID()  { return id; }
protected:
	const NetAddress& addr;
	const UDPSocket& sock;
	ConnectionIncoming in;
	ConnectionOutgoing out;
	Packet buffer;
	
	std::vector<State*> states;
	ServerHandshakeState handshake;
	ServerPlayerGameState game_state;
	uint16_t id;
};

}

#endif
