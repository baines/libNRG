/*
  LibNRG - Networking for Real-time Games

  Copyright (C) 2012-2014 Alex Baines <alex@abaines.me.uk>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#ifndef CONSTANTS_H
#define CONSTANTS_H
#ifdef CLIENTSIDE
#include "SFML/Graphics.hpp"
#endif

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
	static const sf::RectangleShape ball(sf::Vector2f(ball_size, ball_size));
	static const sf::RectangleShape paddle(sf::Vector2f(paddle_w, paddle_h));
#endif
}

#endif
