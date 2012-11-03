#include "nrg.h"
#include <unistd.h>

int main(int argc, char** argv){

	nrg::Client client(nrg::NetAddress("127.0.0.1", "9000"));

	while(true){
		usleep(500000);
		puts("---");
		if(client.update() == nrg::status::ERROR) break;
	}

	return 0;
}