#ifndef CONSTANTS_H
#define CONSTANTS_H
#include "SFML/Graphics.hpp"

namespace constants {

	static const int screen_w = 640;
	static const int screen_h = 480;
	
	static const int ball_size = 16;
	static const float ball_speed = 10.0f;
	static const float ball_speed_inc = 1.0f;
	
	static const int paddle_w = 16;
	static const int paddle_h = 64;
	
	static const int fps_limit = 60;
	static const float lag_scale = 2.0f;
	static const int lag_centre = 64 * lag_scale / 2.0f;
	
	static const char port[] = "9001";
	static const char addr_local[] = "127.0.0.1";
	static const char addr_listen[] = "0.0.0.0";
	
	static const char client_title[] = "NRG Example Game Client";
	static const char replay_arg[] = "--replay";
	
	#ifdef CLIENTSIDE
		static const sf::Color lag_col(0xff, 0xff, 0xff, 0xcc);
		static const sf::Shape ball(sf::Shape::Rectangle(0, 0, ball_size, ball_size, sf::Color::White));
		static const sf::Shape paddle(sf::Shape::Rectangle(0, 0, paddle_w, paddle_h, sf::Color::White));
	#endif
}

#endif
