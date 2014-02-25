#ifndef SERVER_GAME_STATE_H
#define SERVER_GAME_STATE_H
#include "constants.h"
namespace c = constants;

class ServerGameState {
public:
	ServerGameState(nrg::Server& server) : p1(0), p2(c::screen_w-c::paddle_w), ball(), server(server){
		server.registerEntity(p1);
		server.registerEntity(p2);
		server.registerEntity(ball);
	}
	void update(){
		if(server.playerCount() < 2){
			ball.reset();
		} else {
			ball.update();
			if(ball.getX() <= c::paddle_w){
				if(ball.getY() + c::ball_size >= p1.getY() && ball.getY() <= p1.getY() + c::paddle_h){
					ball.setX(c::paddle_w + 1);
					ball.xv *= -1;
					ball.speed += c::ball_speed_inc;
				} else if(ball.getX() <= -c::ball_size) {
					p2.incScore();
					ball.reset();
				}
			}
			if(ball.getX() >= c::screen_w - (c::paddle_w + c::ball_size)){
				if(ball.getY() + c::ball_size > p2.getY() && ball.getY() < p2.getY() + c::paddle_h){
					ball.setX(c::screen_w - (c::paddle_w + c::ball_size + 1));
					ball.xv *= -1;
					ball.speed += c::ball_speed_inc;
				} else if(ball.getX() >= c::screen_w) {
					p1.incScore();
					ball.reset();
				}
			}
			if(ball.getY() <= 0 || ball.getY() >= (c::screen_h - c::ball_size)) ball.yv *= -1;
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
