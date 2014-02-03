.PHONY: clean
CXX:=g++-4.8

all: game_server game_client

game_server: server.cpp example_entity.h ../../libnrg.a
	$(CXX) -std=c++0x -I../../include $< ../../libnrg.a -lrt -lz -o $@ 

game_client: client.cpp example_entity.h sprite.h ../../libnrg.a
	$(CXX) -std=c++0x -I../../include $< ../../libnrg.a -lsfml-graphics -lrt -lz -o $@

clean:
	$(RM) game_server game_client
