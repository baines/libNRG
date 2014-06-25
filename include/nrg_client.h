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
