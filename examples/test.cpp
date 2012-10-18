#include <nrg.h>
#include <cstdio>

int main(int argc, char** argv){

	// test name resolution
	nrg::NetAddress addr("localhost", "9999");
	printf("%s\n", addr.name());

	// test binding socket
	nrg::UDPSocket s;
	s.bind(addr);

	// wait for user input, so that the bound socket can be observed before exit.
	getchar();

	return 0;
}
