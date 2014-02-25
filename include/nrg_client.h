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
#include <utility>

namespace nrg {

class NRG_LIB Client {
public:
	Client(const char* game_name, uint32_t game_version, InputBase& input);
	Client(const char* game_name, uint32_t game_version);

	bool connect(const NetAddress& server_addr);
	bool isConnected() const;
	const NetAddress& getAddress() const;

	void registerEntity(Entity* e);
	
	template<class M, class F>
	void addMessageHandler(const F& f){
		game_state.registerMessage(M(f));
	}
	void sendMessage(const MessageBase& m);
	
	bool update();
	bool pollEvent(Event& e);
	const ClientStats& getStats() const;
	
	void startRecordingReplay(const char* filename);
	void stopRecordingReplay();
	
	InputBase* getInput(){ return input; }
	EventQueue& getEventQueue(){ return eventq; }
	UDPSocket& getSock(){ return sock; }
	
	virtual ~Client();
protected:
	UDPSocket sock;
	InputBase* input;
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
