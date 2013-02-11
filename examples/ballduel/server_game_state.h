#ifndef SERVER_GAME_STATE_H
#define SERVER_GAME_STATE_H

class ServerGameState {
public:
	ServerGameState(nrg::Server& server) : p1(0), p2(624), ball(), server(server){
		server.registerEntity(&p1);
		server.registerEntity(&p2);
		server.registerEntity(&ball);
	}
	void update(){
		if(server.playerCount() < 2) return;
		
	}
	PlayerEntity& getPlayer1() { return p1; }
	PlayerEntity& getPlayer2() { return p2; }
private:
	PlayerEntity p1, p2;
	BallEntity ball;
	nrg::Server& server;
};

#endif
