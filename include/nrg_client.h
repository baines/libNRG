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

class Client {
public:
	Client(const char* game_name, uint32_t game_version, InputBase& input);
	Client(const char* game_name, uint32_t game_version);

	bool connect(const NetAddress& server_addr);
	bool isConnected() const;
	const NetAddress& getAddress() const;

	void registerEntity(Entity* e);

	template<class M, class F>
	void addMessageHandler(F&& f){
		game_state.registerMessage(M(std::forward<F>(f)));
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

	void setUserPointer(void* p){ user_pointer = p; }
	void* getUserPointer() const { return user_pointer; }

	~Client();
private:
	UDPSocket sock;
	InputBase* input;
	Packet buffer;
	NetAddress serv_addr;
	Connection con;
	EventQueue eventq;
	StateManager state_manager;
	ClientHandshakeState handshake;
	ClientGameState game_state;
	void* user_pointer;
	char dc_reason[NRG_MAX_ERRORMSG_LEN];
};

}

#endif
