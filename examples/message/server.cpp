#include "nrg.h"
#include <string>
#include <sstream>
#include <signal.h>

using namespace nrg;
using namespace std;
typedef Message<0, string> TxtMsg;

bool running = true;
void sig(int){
	running = false;
}

int main(void){
	signal(SIGINT, &sig);

	Server server("NRG Message Example", 1);
	
	server.setTickRate(2);

	server.addMessageHandler<TxtMsg>([&](const TxtMsg& m, uint32_t ts, Player* p){
		stringstream ss;
		ss << "<Player " << p->getID() << ">: " << m.get<0>();
		server.broadcastMessage(TxtMsg(ss.str()));
	});

	server.bind(NetAddress("127.0.0.1", "9000"));
	
	while(running && server.update()){
		Event e;
		while(server.pollEvent(e)){
			if(e.type == PLAYER_JOIN){
				stringstream ss;
				ss << "Welcome, Player " << e.player.id << ".";
				server.broadcastMessage(TxtMsg(ss.str()));
			}
			if(e.type == PLAYER_LEAVE){
				stringstream ss;
				ss << "Player " << e.player.id << " has left.";
				server.broadcastMessage(TxtMsg(ss.str()));
			}
		}
	}

	return 0;
}
