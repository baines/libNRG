#include "nrg.h"
#include "entities.h"
#include "server_game_state.h"
#include "input.h"
#include <signal.h>
#include "constants.h"
namespace c = constants;

bool running = true;

void sig(int){
	running = false;
}

int main(int argc, char** argv){
	signal(SIGINT, &sig);

	MyInput input;
	nrg::Server server(nrg::NetAddress(c::addr_listen, c::port), input);
	if(server.isBound()){
		const auto& addr = server.getSocket().getBoundAddress();
		printf("bound to %s:%hu\n", addr->name(), addr->port());
		ServerGameState gs(server);
		input.setGameState(&gs);

		while(running){
			gs.update();
			server.update();
		}
	} else {
		puts("Couldn't bind to port.");
	}

	return 0;
}
