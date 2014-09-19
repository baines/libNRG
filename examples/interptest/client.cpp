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
#include "example_entity.h"
#include "sprite.h"

std::vector<Sprite> sprites;
sf::Text interp_str;
bool running = true;
bool interp = true;
typedef std::vector<Sprite>::iterator s_it;

bool findSprite(nrg::Entity* e, s_it& i){
	return (i = std::find(sprites.begin(), sprites.end(), e)) != sprites.end();
}

void checkNRGEvents(nrg::Client& c){
	nrg::Event e;
	s_it i;

	while(c.pollEvent(e)){
		switch(e.type){
		case nrg::ENTITY_CREATED:
			if(e.entity.etype == EXAMPLE){
				sprites.push_back(Sprite(static_cast<ExampleEntity*>(e.entity.pointer)));
			}
			break;
		case nrg::ENTITY_DESTROYED:
			if(findSprite(e.entity.pointer, i)) sprites.erase(i);
			break;
		case nrg::DISCONNECTED:
			printf("Disconnected. (%s)\n", e.dc.reason);
			running = false;
			break;
		}
	}
}

void updateText(){
	interp_str.setString(interp ? "Interpolation: On." : "Interpolation: Off.");
}

void handleSFMLKeyPress(const sf::Keyboard::Key key){
	switch(key){
	case sf::Keyboard::Escape:
		running = false;
		break;
	case sf::Keyboard::I:
		interp = !interp;
		updateText();
		break;
	}
}

void checkSFMLEvents(sf::RenderWindow& win){
	sf::Event e;
	while(win.pollEvent(e)){
		if(e.type == sf::Event::Closed) running = false;
		if(e.type == sf::Event::KeyPressed) handleSFMLKeyPress(e.key.code);
	}
}

int main(int argc, char** argv){

	nrg::Client client("NRG Interpolation Test", 1);
	client.registerEntity(new ExampleEntity());
	client.connect(nrg::NetAddress("127.0.0.1", "4000"));
	
	sf::RenderWindow window(sf::VideoMode(640, 480), "NRG Example Game Client");
	window.setVerticalSyncEnabled(true);
	
	sf::Font f;
	f.loadFromFile("FreeSans.ttf");
	interp_str.setFont(f);
	
	updateText();

	while(running){
		running = client.update();
		
		checkNRGEvents(client);
		checkSFMLEvents(window);
		
		window.clear();
		window.draw(interp_str);
		for(s_it i = sprites.begin(), j = sprites.end(); i!=j; ++i){
			i->update(interp);	
			i->draw(window);
		}
		window.display();
	}

	window.close();
	
	return 0;
}
