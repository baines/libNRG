#include "nrg.h"
#include "SFML/Graphics.hpp"
#include "example_entity.h"
#include "sprite.h"

std::vector<Sprite> sprites;
sf::Image img;
bool running = true;
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
				sprites.push_back(Sprite(img, static_cast<ExampleEntity*>(e.entity.pointer)));
			}
			break;
		case nrg::ENTITY_UPDATED:
			if(findSprite(e.entity.pointer, i)) i->update();
			break;
		case nrg::ENTITY_DESTROYED:
			if(findSprite(e.entity.pointer, i)) sprites.erase(i);
			break;
		case nrg::DISCONNECTED:
			running = false;
			break;
		}
	}
}

void checkSFMLEvents(sf::RenderWindow& win){
	sf::Event e;
	while(win.GetEvent(e)){
		if(e.Type == sf::Event::Closed || e.Type == sf::Event::KeyPressed
		&& e.Key.Code == sf::Key::Escape){
			 running = false;
		}
	}
}

int main(int argc, char** argv){

	nrg::Client client(nrg::NetAddress("127.0.0.1", "4000"));
	client.registerEntity(new ExampleEntity());

	sf::RenderWindow window(sf::VideoMode(640, 480), "NRG Example Game Client");
	window.UseVerticalSync(true);
	img.Create(16, 16, sf::Color::White);

	while(running){
		if(client.update() != nrg::status::OK) running = false;
		
		checkNRGEvents(client);
		checkSFMLEvents(window);
		
		window.Clear();
		for(s_it i = sprites.begin(), j = sprites.end(); i!=j; ++i) i->draw(window);
		window.Display();
	}

	window.Close();
	
	return 0;
}
