#ifndef NRG_PLAYER_IMPL_H
#define NRG_PLAYER_IMPL_H
#include "nrg_server.h"
#include "nrg_server_state.h"
#include "nrg_state_manager.h"
#include "nrg_connection.h"

namespace nrg {

class PlayerImpl : public Player {
public:
	PlayerImpl(uint16_t id, const Server& server, const NetAddress& addr);
	bool addPacket(Packet& p);
	bool update();
	void kick(const char* reason);
	bool isConnected() const;
	int getPing() const;
	uint16_t getID() const { return id; }
protected:
	const Server& server;
	const NetAddress& addr;
	const UDPSocket& sock;
	Connection con;
	Packet buffer;
	
	int ping;
	StateManager state_manager;
	ServerHandshakeState handshake;
	ServerPlayerGameState game_state;
	uint16_t id;
	bool connected;
};

}

#endif
