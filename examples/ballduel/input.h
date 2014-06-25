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
#ifndef INPUT_H
#define INPUT_H
#include "nrg.h"
#include "server_game_state.h"

class MyInput : public nrg::Input<MyInput> {
public:
	MyInput() : ypos(this) {}
	void setGameState(ServerGameState* gs){
		this->gs = gs;
	}
	void setY(uint16_t y){
		ypos = y;
	}
	void onUpdate(nrg::Player& player){
		if(player.getID() == 0){
			gs->getPlayer1().setY(ypos.get()-(c::paddle_h/2));
		} else if(player.getID() == 1){
			gs->getPlayer2().setY(ypos.get()-(c::paddle_h/2));
		}
	}
private:
	nrg::Field<short> ypos;
	ServerGameState* gs;
};

#endif
