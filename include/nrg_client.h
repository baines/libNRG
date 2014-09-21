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
/** @file
 * Defines the Client class, the main class used by client-side \p nrg programs.
 */
#ifndef NRG_CLIENT_H
#define NRG_CLIENT_H
#include "nrg_core.h"
#include "nrg_version.h"
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

/** The main client-side \p nrg class */
class Client {
public:
	/** Standard Client constructor
	 *  @param game_game User-chosen game name, it must match the Server's game name
	 *  @param game_version User-chosen game version
	 *  @param input User-defined subclass of InputBase, containing data to be sent to the server each frame
	 */
	Client(const std::string& game_name, uint32_t game_version, InputBase& input);
	
	/** No-Input Client constructor
	 *  @param game_game User-chosen game name, it must match the Server's game name
	 *  @param game_version User-chosen game version
	 */
	Client(const std::string& game_name, uint32_t game_version);

	/** Sets the address of the Server to connect to, and calls Socket::connect on it */
	bool connect(const NetAddress& server_addr);
	
	/** Returns whether or not this Client is connected or not */
	bool isConnected() const;
	
	/** Returns the NetAddress this Client is connected to, or \ref nullptr if not connected */
	const NetAddress* getConnectedAddress() const;

	/** Registers a subclass of Entity with this Client, must be called once for each user-defined Entity before connecting */
	void registerEntity(Entity* e);

	/** Adds a function \p f, that will be called when the Client receives a Message of type \p M */
	template<class M, class F>
	void addMessageHandler(F&& f){
		game_state.registerMessage(M(std::forward<F>(f)));
	}

	/** Queues the the user-defined Message \p m to be sent to the connected Server */
	void sendMessage(const MessageBase& m);

	/** Sends and Recieves any queued Packets and updates the contained State classes, returns false on error */
	bool update();
	
	/** Places the next Event from the Client's EventQueue into \p e, returning true if this happened, or false if there were no more events */
	bool pollEvent(Event& e);
	
	/** Returns a ClientStats reference, which contains statistics about this Client's connection */
	const ClientStats& getStats() const;
	
	/** @{ */
	/** Gets a limit on how many full Packets the Client will send per second */
	uint32_t getPacketRateLimit(void);
	/** Sets a limit on how many full Packets the Client will send per second */
	void setPacketRateLimit(uint32_t packets_per_sec);
	/** @} */

	/** Starts recording received packets into a file called \p filename */
	void startRecordingReplay(const char* filename);
	/** Stops recording received packets into a file called \p filename */
	void stopRecordingReplay();

	/** @{ */
	/** Get a user-defined pointer to be associated with this Client instance */
	void* getUserPointer() const  { return user_pointer; }
	/** Set a user-defined pointer to be associated with this Client instance */
	void  setUserPointer(void* p) { user_pointer = p; }
	/** @} */

	/** @name Misc Getters */
	/** @{ */
	InputBase*         getInput()       { return input; }
	EventQueue&        getEventQueue()  { return eventq; }
	UDPSocket&         getSock()        { return sock; }
	const std::string& getGameName()    { return game_name; }
	const uint32_t     getGameVersion() { return game_version; }
	const int          getPlayerID()    { return player_id; }
	/** @} */
	
	/** @cond INTERNAL_USE_ONLY */
	void setServerParams(const Version& lib_v, uint32_t game_v, uint16_t pid); 
	/** @endcond */

	/** Standard destructor */
	~Client();
private:
	UDPSocket sock;
	InputBase* input;
	Packet buffer;
	NetAddress serv_addr;
	Connection con;
	StateConnectionOutImpl state_con;
	EventQueue eventq;
	StateManager state_manager;
	ClientHandshakeState handshake;
	ClientGameState game_state;
	uint32_t rate_limit_interval_ms;
	uint32_t previous_ms;
	std::string game_name;
	uint32_t game_version;
	int player_id;
	void* user_pointer;
	char dc_reason[NRG_MAX_ERRORMSG_LEN];
};

}

#endif
