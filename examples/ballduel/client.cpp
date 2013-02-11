#include "nrg.h"
#include "SFML/Graphics.hpp"
#include "entities.h"
#include "input.h"
#include "sprite.h"

std::vector<Sprite> sprites;
sf::Image img;
bool running = true;
bool interp = true;
typedef std::vector<Sprite>::iterator s_it;
MyInput input;

bool findSprite(nrg::Entity* e, s_it& i){
	return (i = std::find(sprites.begin(), sprites.end(), e)) != sprites.end();
}

void checkNRGEvents(nrg::Client& c){
	nrg::Event e;
	s_it i;

	while(c.pollEvent(e)){
		switch(e.type){
		case nrg::ENTITY_CREATED:
			if(e.entity.etype == PLAYER){
				sprites.push_back(Sprite(img, static_cast<PlayerEntity*>(e.entity.pointer)));
			} else if(e.entity.etype == BALL){
				sprites.push_back(Sprite(img, static_cast<BallEntity*>(e.entity.pointer)));
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

void handleSFMLKeyPress(const sf::Key::Code key){
	switch(key){
	case sf::Key::Escape:
		running = false;
		break;
	}
}

void checkSFMLEvents(sf::RenderWindow& win){
	sf::Event e;
	while(win.GetEvent(e)){
		if(e.Type == sf::Event::Closed) running = false;
		if(e.Type == sf::Event::KeyPressed) handleSFMLKeyPress(e.Key.Code);
		if(e.Type == sf::Event::MouseMoved) input.setY(e.MouseMove.Y);
	}
}

int main(int argc, char** argv){

	nrg::Client client(nrg::NetAddress("127.0.0.1", "4000"), input);
	client.registerEntity(new PlayerEntity(0));
	client.registerEntity(new BallEntity());

	sf::RenderWindow window(sf::VideoMode(640, 480), "NRG Example Game Client");
	window.UseVerticalSync(true);
	img.Create(16, 16, sf::Color::White);

	while(running){
		if(client.update() != nrg::status::OK) running = false;
		
		checkNRGEvents(client);
		checkSFMLEvents(window);

		printf("%lu\n", sprites.size());
		
		window.Clear();
		for(s_it i = sprites.begin(), j = sprites.end(); i!=j; ++i){
			i->update();	
			i->draw(window);
		}
		window.Display();
	}

	window.Close();
	
	return 0;
}
