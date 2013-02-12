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
		if(server.playerCount() < 2){
			ball.reset();
		} else {
			ball.update();
			if(ball.getX() <= 16){
				if(ball.getY() + 16 > p1.getY() && ball.getY() < p1.getY() + 64){
					ball.setX(17);
					ball.xv *= -1;
					ball.speed += 0.1f;
				} else if(ball.getX() <= -16) {
					p2.incScore();
					ball.reset();
				}
			}
			if(ball.getX() >= 608){
				if(ball.getY() + 16 > p2.getY() && ball.getY() < p2.getY() + 64){
					ball.setX(607);
					ball.xv *= -1;
					ball.speed += 0.1f;
				} else if(ball.getX() >= 640) {
					p1.incScore();
					ball.reset();
				}
			}
			if(ball.getY() <= 0 || ball.getY() >= 466) ball.yv *= -1;
		}
	}
	PlayerEntity& getPlayer1() { return p1; }
	PlayerEntity& getPlayer2() { return p2; }
private:
	PlayerEntity p1, p2;
	BallEntity ball;
	nrg::Server& server;
};

#endif
