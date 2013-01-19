#include "nrg.h"
#include "example_entity.h"
bool running = true;
int t = 100;

int main(int argc, char** argv){

	nrg::Server server(nrg::NetAddress("127.0.0.1", "4000"));
	
	ExampleEntity* e = new ExampleEntity();

	server.registerEntity(e);

	while(running){
		if(e){
			e->update();
		}
		server.update();
		if(e && (t-- < 0)){
			delete e;
			e = NULL;
			puts("BOOM");
		}
	}
	
}
