.PHONY: clean

all: game_server game_client

game_server: server.cpp entities.h server_game_state.h input.h ../../libnrg.a
	$(CXX) -std=c++0x -g -I../../include $< ../../libnrg.a -lrt -lz -o $@ 

game_client: client.cpp entities.h sprite.h input.h ../../libnrg.a
	$(CXX) -DCLIENTSIDE -std=c++0x -g -I../../include $< ../../libnrg.a -lsfml-graphics -lsfml-window -lsfml-system -lrt -lz -o $@

clean:
	$(RM) game_server game_client
