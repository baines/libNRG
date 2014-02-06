#ifndef NRG_CLIENT_H
#define NRG_CLIENT_H
#include "nrg_core.h"
#include "nrg_socket.h"
#include "nrg_netaddress.h"
#include "nrg_client_state.h"
#include "nrg_state_manager.h"
#include "nrg_event.h"
#include "nrg_input.h"
#include "nrg_message.h"
#include <vector>

namespace nrg {

class NRG_LIB Client {
public:
	Client(Input& input = null_input);
	Client(const NetAddress& server_addr, Input& input = null_input);
	~Client();
	bool connect(const NetAddress& server_addr);
	bool isConnected() const;
	const NetAddress& getAddress() const;

	void registerEntity(Entity* e);
	void registerMessage(const MessageBase& m);
	
	bool update();
	bool pollEvent(Event& e);
	const ClientStats& getStats() const;
	
	void startRecordingReplay(const char* filename);
	void stopRecordingReplay();
protected:
	UDPSocket sock;
	Input& input;
	Packet buffer;
	NetAddress serv_addr;
	Connection con;
	EventQueue eventq;
	StateManager state_manager;
	ClientHandshakeState handshake;
	ClientGameState game_state;
	char dc_reason[NRG_MAX_ERRORMSG_LEN];
};

}

#endif
