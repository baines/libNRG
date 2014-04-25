#ifndef NRG_PLAYER_H
#define NRG_PLAYER_H
#include "nrg_core.h"
#include "nrg_message.h"
namespace nrg {

class Server;

struct Player {
	virtual uint16_t getID() const = 0;
	virtual void kick(const char* reason) = 0;
	virtual const Server* getServer() const = 0;
	virtual const NetAddress& getRemoteAddress() const = 0;
	virtual void registerMessageHandler(MessageBase&&) = 0;
	virtual void registerMessageHandler(const MessageBase&) = 0;
	virtual void sendMessage(const MessageBase& m) = 0;
	virtual bool isConnected() const = 0;
	virtual int getPing() const = 0;
	virtual ~Player(){}
};

}

#endif
