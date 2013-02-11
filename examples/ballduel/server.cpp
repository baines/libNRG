#include "nrg.h"
#include "entities.h"
#include "server_game_state.h"
#include "input.h"
#include <signal.h>

bool running = true;

void sig(int){
	running = false;
}

int main(int argc, char** argv){
	signal(SIGINT, &sig);

	MyInput input;
	nrg::Server server(nrg::NetAddress("127.0.0.1", "4000"), input);
	ServerGameState gs(server);
	input.setGameState(&gs);

	while(running){
		gs.update();
		server.update();
	}

	return 0;
}
