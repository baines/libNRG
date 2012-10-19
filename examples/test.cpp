#include <nrg.h>
#include <cstdio>

int main(int argc, char** argv){

	// test name resolution
	nrg::NetAddress addr("localhost", "9999");
	// should print 127.0.0.1
	printf("%s\n", addr.name());

	// test binding socket
	nrg::UDPSocket s;
	s.bind(addr);

	// should print 127.0.0.1
	printf("%s\n", s.getBoundAddress()->name());

	// should print 9999
	printf("%u\n", addr.port()); 

	// wait for user input, so that the bound socket can be observed before exit.
	getchar();

	return 0;
}
