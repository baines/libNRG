#include "nrg.h"
#include "example_entity.h"

int main(int argc, char** argv){

	nrg::Client client(nrg::NetAddress("127.0.0.1", "4000"));
	nrg::Event e;

	client.registerEntity(new ExampleEntity());

	while(true){
		client.update();
		while(client.pollEvent(e)){
			if(e.type == nrg::ENTITY_UPDATED && e.entity.etype == EXAMPLE){
				ExampleEntity* ee = static_cast<ExampleEntity*>(e.entity.pointer);
				printf("%d %d\n", ee->getx(), ee->gety());
			}

		}
		usleep(10000);
	}
	
	return 0;

}
