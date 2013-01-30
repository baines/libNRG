#include "nrg.h"
#include "example_entity.h"
bool running = true;

int main(int argc, char** argv){

	nrg::Server server(nrg::NetAddress("127.0.0.1", "4000"));
	
	ExampleEntity e;

	server.registerEntity(&e);

	while(running){
		e.update();
		server.update();
	}
	
}
