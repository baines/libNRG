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
#include "nrg.h"
#include "SFML/Graphics.hpp"
#include "entities.h"
#include "input.h"
#include "sprite.h"
#include "constants.h"
#include "client.h"
namespace c = constants;

bool running = true;
MyInput input;
std::vector<Sprite> sprites;
int score1 = 0, score2 = 0;
sf::Text score_str;

void clientAddEntity(PlayerEntity* e){ sprites.push_back(Sprite(e)); }
void clientAddEntity(BallEntity* e){ sprites.push_back(Sprite(e)); }
void clientDelEntity(uint16_t id){
	sprites.erase(std::remove(sprites.begin(), sprites.end(), id), sprites.end());
}
void clientSetScore(bool left_player, int score){
	int& i = (left_player) ? score1 : score2;
	i = score;

	char buf[32];
	snprintf(buf, sizeof(buf), "%3d  -  %-3d", score1, score2);
	score_str.setString(buf);
	score_str.setOrigin(score_str.getGlobalBounds().width/2, 0);
}

void checkNRGEvents(nrg::Client& c){
	nrg::Event e;

	while(c.pollEvent(e)){
		if(e.type == nrg::DISCONNECTED){
			printf("Disconnected. (%s)\n", e.dc.reason);
			running = false;
		}
	}
}

void handleSFMLKeyPress(const sf::Keyboard::Key key){
	switch(key){
	case sf::Keyboard::Escape:
		running = false;
		break;
	}
}

void checkSFMLEvents(sf::RenderWindow& win){
	sf::Event e;
	while(win.pollEvent(e)){
		if(e.type == sf::Event::Closed) running = false;
		if(e.type == sf::Event::KeyPressed) handleSFMLKeyPress(e.key.code);
		if(e.type == sf::Event::MouseMoved){ 
			sf::Vector2i m_pos(e.mouseMove.x, e.mouseMove.y);
			input.setY(win.mapPixelToCoords(m_pos).y);
		}
	}
}

int main(int argc, char** argv){
	nrg::ReplayServer rserv;
	bool playing_replay = false;

	if(argc > 2 && strcmp(argv[1], c::replay_arg) == 0){
		playing_replay = true;
		rserv.openReplay(argv[2]);
		rserv.bind(c::port);
	}

	nrg::Client client("NRG Ball Duel", 1, input);
	client.registerEntity(new PlayerEntity(0));
	client.registerEntity(new BallEntity());
	client.connect(nrg::NetAddress((playing_replay || argc < 2) ? c::addr_local : argv[1], c::port));

	sf::RenderWindow window(sf::VideoMode(c::screen_w, c::screen_h), c::client_title);
	window.setVerticalSyncEnabled(true);
	window.setFramerateLimit(c::fps_limit);

	sf::Font score_font;
	score_font.loadFromFile("FreeSans.ttf");

	score_str.setFont(score_font);
	score_str.setPosition(c::screen_w / 2, 0);

	uint32_t tex[64*64];

	sf::Texture lagometer;
	lagometer.create(64, 64);
	lagometer.setSmooth(false);

	sf::Sprite lsprite(lagometer);
	lsprite.setColor(c::lag_col);
	lsprite.setScale(c::lag_scale, c::lag_scale);
	lsprite.setOrigin(c::lag_centre, c::lag_centre);
	lsprite.setPosition(c::screen_w, c::screen_h);

	if(!playing_replay && argc > 2) client.startRecordingReplay(argv[2]);

	while(running){
		if(playing_replay) rserv.update();
		running = client.update();

		lagometer.update(client.getStats().toRGBATexture(tex));
		checkNRGEvents(client);
		checkSFMLEvents(window);

		window.clear();
		for(auto& s : sprites){
			s.draw(window);
		}
		window.draw(lsprite);
		window.draw(score_str);
		window.display();
	}

	window.close();

	return 0;
}
