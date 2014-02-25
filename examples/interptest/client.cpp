#include "nrg.h"
#include "SFML/Graphics.hpp"
#include "example_entity.h"
#include "sprite.h"

std::vector<Sprite> sprites;
sf::Image img;
sf::String interp_str;
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
				sprites.push_back(Sprite(img, static_cast<ExampleEntity*>(e.entity.pointer)));
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
	interp_str.SetText(interp ? "Interpolation: On." : "Interpolation: Off.");
}

void handleSFMLKeyPress(const sf::Key::Code key){
	switch(key){
	case sf::Key::Escape:
		running = false;
		break;
	case sf::Key::I:
		interp = !interp;
		updateText();
		break;
	}
}

void checkSFMLEvents(sf::RenderWindow& win){
	sf::Event e;
	while(win.GetEvent(e)){
		if(e.Type == sf::Event::Closed) running = false;
		if(e.Type == sf::Event::KeyPressed) handleSFMLKeyPress(e.Key.Code);
	}
}

int main(int argc, char** argv){

	nrg::Client client("NRG Interpolation Test", 1);
	client.registerEntity(new ExampleEntity());
	client.connect(nrg::NetAddress("127.0.0.1", "4000"));
	
	sf::RenderWindow window(sf::VideoMode(640, 480), "NRG Example Game Client");
	window.UseVerticalSync(true);
	img.Create(16, 16, sf::Color::White);
	updateText();

	while(running){
		running = client.update();
		
		checkNRGEvents(client);
		checkSFMLEvents(window);
		
		window.Clear();
		window.Draw(interp_str);
		for(s_it i = sprites.begin(), j = sprites.end(); i!=j; ++i){
			i->update(interp);	
			i->draw(window);
		}
		window.Display();
	}

	window.Close();
	
	return 0;
}
