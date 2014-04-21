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
	
	client.addMessageHandler<TxtMsg>([](const TxtMsg& m, uint32_t ts){
		cout << "[From Server]: " << m.get<0>() << endl;
	});
	
	client.connect(NetAddress("127.0.0.1", "9000"));
	
	while(running && client.isConnected()){
		client.update();
		
		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 500000;
		
		int fd = fileno(stdin);
		
		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(fd, &fds);
		
		if(select(fd+1, &fds, nullptr, nullptr, &tv) == 1){
			string input;
			getline(cin, input);
			client.sendMessage(TxtMsg(input));
		}
	}

	return 0;
}
