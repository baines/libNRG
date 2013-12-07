.PHONY: clean

all: game_server game_client

game_server: server.cpp ../../libnrg.a
	$(CXX) -I../../include $< ../../libnrg.a -lrt -lz -o $@ 

game_client: client.cpp ../../libnrg.a
	$(CXX) -I../../include $< ../../libnrg.a -lsfml-graphics -lrt -lz -o $@

clean:
	$(RM) game_server game_client
