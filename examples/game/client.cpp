#include "nrg.h"

int main(int argc, char** argv){

	nrg::Client client(nrg::NetAddress("127.0.0.1", "4000"));
	
	return 0;

}
