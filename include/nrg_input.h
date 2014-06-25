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
#ifndef NRG_INPUT_H
#define NRG_INPUT_H
#include "nrg_core.h"
#include "nrg_field.h"
#include "nrg_player.h"
#include <utility>
#include <functional>
#include <vector>

namespace nrg {

struct InputBase : protected FieldContainer {
	void markUpdated(bool b);
	bool readFromPacket(Packet& p);
	void writeToPacket(Packet& p) const;

	virtual void onUpdate(Player& player) = 0;	
	virtual void doPrediction() = 0;
};

template<class CRTP>
struct Input : public InputBase {
	void addPredictionFunc(std::function<void(CRTP&)>&& func){
		predict_funcs.push_back(std::move(func));
	}
	void doPrediction(){
		for(auto& f : predict_funcs){
			f(*static_cast<CRTP* const>(this));
		}
	}
private:
	std::vector<std::function<void(CRTP&)>> predict_funcs;
};

}

#endif
