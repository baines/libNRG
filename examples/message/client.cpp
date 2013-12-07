#include "nrg.h"

int main(void){

	using namespace nrg;
	
	Client client(NetAddress("127.0.0.1", "9000"));
	
	typedef Message<0, int, float, uint8_t> TestMsg;
	
	client.registerMessage(TestMsg([](const TestMsg& m){
		std::cout << "Got Test message: " << std::endl
		          << m.get<0>() << std::endl
		          << m.get<1>() << std::endl
		          << m.get<2>() << std::endl;
	}));

	return 0;
}
