#include "nrg_state.h"

void nrg::ServerMasterGameState::registerEntity(Entity* e){
	e->nrg_serv_ptr = this;
//	e->nrg_id = getNextEntityId();
}

void nrg::ServerMasterGameState::markEntityUpdated(Entity* e){
	updated_entities.push_back(e);
}

nrg::ServerMasterGameState::~ServerMasterGameState(){
	for(std::vector<Entity*>::iterator i = entities.begin(), j = entities.end()
	; i != j; ++i){
		if((*i)){
			(*i)->nrg_serv_ptr = NULL;
			(*i)->nrg_id = 0;
		}
	}
}
