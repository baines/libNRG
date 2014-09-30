/*
  LibNRG - Networking for Real-time Games

  Copyright (C) 2012-2014 Alex Baines <alex@abaines.me.uk>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include <nrg.h>
#include <cstdio>

int main(int argc, char** argv){

	// test name resolution
	nrg::NetAddress addr("localhost", "9999");
	// should print 127.0.0.1
	printf("%s\n", addr.name());

	// test operator== and !=, should print 1 0
	nrg::NetAddress addr2("127.0.0.1", "9999");
	printf("%d %d\n", addr == addr2, addr != addr2);

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
