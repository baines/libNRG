#include "nrg.h"
#include "SFML/Graphics.hpp"
#include "entities.h"
#include "input.h"
#include "sprite.h"
#include "constants.h"
namespace c = constants;

std::vector<Sprite> sprites;
bool running = true;
typedef std::vector<Sprite>::iterator s_it;
MyInput input;
int score1 = 0, score2 = 0;
sf::String score_str;

bool findSprite(nrg::Entity* e, s_it& i){
	return (i = std::find(sprites.begin(), sprites.end(), e)) != sprites.end();
}

void updateScore(){
	char buf[32];
	snprintf(buf, sizeof(buf), "%3d  -  %-3d", score1, score2);
	score_str.SetText(buf);
	score_str.SetCenter(score_str.GetRect().GetWidth()/2, 0);
}

void checkNRGEvents(nrg::Client& c){
	nrg::Event e;
	s_it i;

	while(c.pollEvent(e)){
		switch(e.type){
		case nrg::ENTITY_CREATED:
			if(e.entity.etype == PLAYER){
				sprites.push_back(Sprite(static_cast<PlayerEntity*>(e.entity.pointer)));
			} else if(e.entity.etype == BALL){
				sprites.push_back(Sprite(static_cast<BallEntity*>(e.entity.pointer)));
			}
			break;
		case nrg::ENTITY_DESTROYED:
			if(findSprite(e.entity.pointer, i)) sprites.erase(i);
			break;
		case nrg::ENTITY_UPDATED:
			if(e.entity.etype == PLAYER){
				PlayerEntity* p = static_cast<PlayerEntity*>(e.entity.pointer);
				if(p->getX() < c::screen_w / 2){
					score1 = p->getScore();
				} else {	
					score2 = p->getScore();
				}
				updateScore();
			}
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
		if(e.Type == sf::Event::MouseMoved){ input.setY(e.MouseMove.Y); }
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

	nrg::Client client(nrg::NetAddress((playing_replay || argc < 2) ? c::addr_local : argv[1], c::port), input);
	client.registerEntity(new PlayerEntity(0));
	client.registerEntity(new BallEntity());

	sf::View view(sf::FloatRect(0, 0, c::screen_w, c::screen_h));
	sf::RenderWindow window(sf::VideoMode(c::screen_w, c::screen_h), c::client_title);
	window.SetView(view);
	window.UseVerticalSync(true);
	window.SetFramerateLimit(c::fps_limit);
	
	score_str.SetX(c::screen_w / 2);

	uint32_t tex[64*64];

	sf::Image lagometer;
	lagometer.SetSmooth(false);
	lagometer.LoadFromPixels(64, 64, client.getStats().toRGBATexture(tex));
	sf::Sprite lsprite(lagometer);
	lsprite.SetColor(c::lag_col);
	lsprite.SetScale(c::lag_scale, c::lag_scale);
	lsprite.SetCenter(c::lag_centre, c::lag_centre);
	lsprite.SetPosition(c::screen_w, c::screen_h);

	if(!playing_replay && argc > 2) client.startRecordingReplay(argv[2]);
	
	while(running){
		if(playing_replay) rserv.update();
		running = client.update();
		
		lagometer.LoadFromPixels(64, 64, client.getStats().toRGBATexture(tex));
		checkNRGEvents(client);
		checkSFMLEvents(window);
		
		window.Clear();
		for(s_it i = sprites.begin(), j = sprites.end(); i!=j; ++i){
			i->draw(window);
		}
		window.Draw(lsprite);
		window.Draw(score_str);
		window.Display();
	}

	window.Close();
	
	return 0;
}
