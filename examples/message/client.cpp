#include "nrg.h"
#include <iostream>

int main(void){

	using namespace nrg;
	
	Client client("NRG Message example", 1);
	
	typedef Message<0, int, float, uint8_t> TestMsg;
	
	client.addMessageHandler<TestMsg>([](const TestMsg& m){
		std::cout << "Got Test message: " << std::endl
		          << m.get<0>() << std::endl
		          << m.get<1>() << std::endl
		          << m.get<2>() << std::endl;
	});
	
	client.connect(NetAddress("127.0.0.1", "9000"));

	return 0;
}
