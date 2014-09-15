.PHONY: all win clean

CXXFLAGS := -fno-rtti -std=c++11 -g -I../../include

all: game_server game_client

win: game_server.exe game_client.exe

game_server: server.cpp entities.h server_game_state.h input.h constants.h ../../libnrg.a
	$(CXX) $(CXXFLAGS) $< ../../libnrg.a -lrt -lz -o $@ 

game_client: client.cpp entities.h sprite.h input.h constants.h ../../libnrg.a
	$(CXX) $(CXXFLAGS) -DCLIENTSIDE $< ../../libnrg.a -lsfml-graphics -lsfml-window -lsfml-system -lrt -lz -o $@

game_server.exe: server.cpp entities.h server_game_state.h input.h constants.h ../../libnrg.mingw.a
	i686-w64-mingw32-g++ $(CXXFLAGS) $< ../../libnrg.mingw.a -lws2_32 -o $@

game_client.exe: client.cpp entities.h sprite.h input.h constants.h ../../libnrg.mingw.a
	i686-w64-mingw32-g++ $(CXXFLAGS) -DCLIENTSIDE $< ../../libnrg.mingw.a -lsfml-graphics -lsfml-window -lsfml-system -lws2_32 -o $@

clean:
	$(RM) game_server game_client
