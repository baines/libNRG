#ifndef SPRITE_H
#define SPRITE_H

class Sprite {
public:
	Sprite(sf::Image i, ExampleEntity* e) : s(i), entity(e){ 
		update(true);
	}
	void update(bool interp){
		s.SetPosition(entity->getX(interp), entity->getY(interp));
	}
	void draw(sf::RenderWindow& win){
		win.Draw(s);
	}
	bool operator==(nrg::Entity* e){
		if(e->getType() != entity->getType()){
			return false;
		} else {
			return entity == static_cast<ExampleEntity*>(e);
		}
	}
private:
	sf::Sprite s;
	ExampleEntity* entity;
};


#endif
