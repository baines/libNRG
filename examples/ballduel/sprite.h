#ifndef SPRITE_H
#define SPRITE_H

class Sprite {
public:
	Sprite(sf::Image& i, BallEntity* e) : s(i), entity(e), type(BALL) {
	}
	Sprite(sf::Image& i, PlayerEntity* e) : s(i), entity(e), type(PLAYER) {
		s.SetScaleY(4.0f);
	}
	void draw(sf::RenderWindow& win){
		win.Draw(s);
	}
	void update(){
		s.SetPosition(entity->getX(), entity->getY());
	}
	bool operator==(nrg::Entity* e){
		return e->getID() == entity->getID();
	}
private:
	sf::Sprite s;
	EntityBase* entity;
	uint16_t type;
};


#endif
