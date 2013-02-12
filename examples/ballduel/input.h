#ifndef INPUT_H
#define INPUT_H
#include "nrg.h"
#include "server_game_state.h"

class MyInput : public nrg::Input {
public:
	MyInput() : ypos(this) {}
	void getFields(nrg::FieldList& fl){
		fl.add(ypos);
	}
	void setGameState(ServerGameState* gs){
		this->gs = gs;
	}
	void setY(uint16_t y){
		ypos = y;
	}
	void onUpdateNRG(nrg::Player& player){
		if(player.getID() == 0){
			gs->getPlayer1().setY(ypos.get());
		} else if(player.getID() == 1){
			gs->getPlayer2().setY(ypos.get());
		}
	}
private:
	nrg::Field<short> ypos;
	ServerGameState* gs;
};

#endif
