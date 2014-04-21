.PHONY: clean
CXX:=g++-4.7
CXXFLAGS:=-std=c++0x -U_FORTIFY_SOURCE -fno-stack-protector -fno-rtti -fno-exceptions -O0 -g -I../../include
all: game_client game_server


game_server: server.cpp ../../libnrg.a
	$(CXX) $(CXXFLAGS) $< ../../libnrg.a -lrt -lz -o $@ 

game_client: client.cpp ../../libnrg.a
	$(CXX) $(CXXFLAGS) $< ../../libnrg.a -lrt -lz -o $@

clean:
	$(RM) game_server game_client
