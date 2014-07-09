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
#include <string>
#include <iostream>
#include <signal.h>

using namespace nrg;
using namespace std;
typedef Message<0, string> TxtMsg;

bool running = true;

void sig(int){
	running = false;
}

int main(void){
	signal(SIGINT, &sig);
	
	Client client("NRG Message Example", 1);
	
	client.addMessageHandler<TxtMsg>([&](const TxtMsg& m, uint32_t ts){
		cout << "\n" << m.get<0>() << "\n> " << flush;
	});
	
	client.connect(NetAddress("127.0.0.1", "9000"));
	
	while(running && client.isConnected()){
		client.update();
		
		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 500000;
		
		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(STDIN_FILENO, &fds);
		
		if(select(STDIN_FILENO+1, &fds, nullptr, nullptr, &tv) == 1){
			string input;
			if(getline(cin, input)){
				client.sendMessage(TxtMsg(input));
			}
		}
	}

	return 0;
}
