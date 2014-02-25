#ifndef NRG_INPUT_H
#define NRG_INPUT_H
#include "nrg_core.h"
#include "nrg_field.h"
#include "nrg_player.h"
#include <utility>
#include <functional>
#include <vector>

namespace nrg {

struct NRG_LIB InputBase : public FieldContainer {
	void markUpdated(bool b);
	bool readFromPacket(Packet& p);
	void writeToPacket(Packet& p) const;

	virtual void onUpdate(Player& player) = 0;	
	virtual void doPrediction() = 0;
};

template<class CRTP>
struct NRG_LIB Input : public InputBase {
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
