#include "nrg.h"
#include "example_entity.h"
#include <signal.h>

bool running = true;

void sig(int){
	running = false;
}

int main(int argc, char** argv){
	signal(SIGINT, &sig);
	nrg::Server server(nrg::NetAddress("127.0.0.1", "4000"));
	
	ExampleEntity e;

	server.registerEntity(&e);

	while(running){
		e.update();
		server.update();
	}
	
}
