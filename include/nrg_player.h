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
 *  Contains the Player Interface used server-side
 */
#ifndef NRG_PLAYER_H
#define NRG_PLAYER_H
#include "nrg_core.h"
#include "nrg_message.h"
namespace nrg {

class Server;

/** Represents a connected client on the server */
struct Player {
	/** Returns the library-assigned player's ID */
	virtual uint16_t getID() const = 0;

	/** Kick the player from the server, sending them a reason why */
	virtual void kick(const char* reason) = 0;

	/** Returns the server associated with this Player */
	virtual const Server* getServer() const = 0;

	/** Returns the address the Client associated with this Player is connecting from */
	virtual const NetAddress& getRemoteAddress() const = 0;

	/** Register a Message containing a callback with this Player (R-Value reference version) */
	virtual void registerMessageHandler(MessageBase&&) = 0;

	/** Register a Message containing a callback with this Player (const reference version) */
	virtual void registerMessageHandler(const MessageBase&) = 0;

	/** Send the player a Message with the contents specified in \p m */
	virtual void sendMessage(const MessageBase& m) = 0;

	/** Returns true if this player is connected to a Server */
	virtual bool isConnected() const = 0;

	/** Returns the round-trip latency of this player */
	virtual int getPing() const = 0;

	/** Default destructor */
	virtual ~Player(){}
};

}

#endif
