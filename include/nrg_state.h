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
#ifndef NRG_STATE_H
#define NRG_STATE_H
#include "nrg_core.h"
#include "nrg_connection.h"
#include "nrg_packet.h"
/** @file
 *  Functionality related to the State abstract class
 */
namespace nrg {

/** Enumeration returned by State::Update to explain how the update went */
enum StateResult : uint32_t {
	STATE_CONTINUE = 0x00,
	STATE_EXIT_BIT = 0x10,
	STATE_FAILURE  = STATE_EXIT_BIT,
	STATE_CHANGE   = STATE_EXIT_BIT | 0x01
};

/** Enumeration passed into State::Update to tell why the update is occuring */
enum StateFlags : uint32_t {
	SFLAG_NONE      = 0x00,
	SFLAG_TIMED_OUT = 0x01
};

/** Enumeration for ClientHandshakeState and ServerHandshakeState */
enum HS_Reponses : int8_t {
	HS_WRONG_LIB_VERSION  = -1,
	HS_WRONG_GAME         = -2,
	HS_WRONG_GAME_VERSION = -3,
	HS_SERVER_FULL        = -4,
	HS_NONE               = 0,
	HS_ACCEPTED           = 1
};

class Client;
class Server;
class Player;

/** Interface for wrapping ConnectionOut with rate-limiting functionality */
struct StateConnectionOut {
	/** Returns true if more Packets can be sent right now with sendPacket */
	virtual bool ready() = 0;

	/** Try to send a packet, returns a error Status if it couldn't because of the rate-limit */
	virtual Status sendPacket(Packet& p, PacketFlags f = PKTFLAG_NONE) = 0;

	/** Try to resend the last packet send, returns an error status if it couldn't because of the rate-limit */
	virtual Status resendLastPacket() = 0;
};

/** Abstract class that represents a protocol followed by the Server and Client */
struct State {
	/** Called to (re)initialise this State instance, for Client-side \p c won't be null, but \p s and \p p will, opposite is true for Server-side */
	virtual bool init(Client* c, Server* s, Player* p) = 0;

	/** Called when a Packet is recieved and this is the active State, \p f shows if the packet was out-of-order or not */
	virtual bool onRecvPacket(Packet& p, PacketFlags f) = 0;

	/** Called to see if the State would like to be updated */
	virtual bool needsUpdate() const = 0;

	/** Returns a time that the state can go without needsUpdate() returning true, after which update() will be called with \p f set to STATE_TIMED_OUT */
	virtual size_t getTimeoutSeconds() const { return 10; }

	/** Called to update the state (or on a timeout) allowing it to optionally send some Packets using \p out */
	virtual StateResult update(StateConnectionOut& out, StateFlags f = SFLAG_NONE) = 0;

	/** Default destructor */
	virtual ~State(){}
};

/** Implementation of StateConnectionOut */
struct StateConnectionOutImpl : StateConnectionOut {
	/** Default constructor */
	StateConnectionOutImpl(ConnectionOut& out);

	/** Marks the StateConnectionOutImpl so that \p ready_to_send decides if more packets can be sent or not */
	void reset(bool ready_to_send);

	/** Returns true if one or more packets have been sent through this StateConnectionOutImpl since the last reset() */
	bool sentPackets();

	bool ready();
	Status sendPacket(Packet& p, PacketFlags f = PKTFLAG_NONE);
	Status resendLastPacket();
private:
	ConnectionOut& out;
	bool isready, sent;
};


}

#endif
