.PHONY: clean

all: game_server game_client

game_server: server.cpp example_entity.h ../../libnrg.a
	$(CXX) -I../../include $< ../../libnrg.a -lrt -o $@ 

game_client: client.cpp example_entity.h ../../libnrg.a
	$(CXX) -I../../include $< ../../libnrg.a -lrt -o $@

clean:
	$(RM) game_server game_client
