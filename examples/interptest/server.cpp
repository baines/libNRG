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
#include "nrg.h"
#include "example_entity.h"
#include <signal.h>

bool running = true;

void sig(int){
	running = false;
}

int main(int argc, char** argv){
	signal(SIGINT, &sig);
	nrg::Server server("NRG Interpolation Test", 1);
	server.bind(nrg::NetAddress("127.0.0.1", "4000"));
	
	ExampleEntity e;

	server.registerEntity(e);

	while(running){
		e.update();
		server.update();
	}
	
}
