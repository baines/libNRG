#include "nrg_entity.h"
#include "nrg_state.h"

nrg::Entity::Entity() : nrg_id(0), nrg_updated(false), nrg_serv_ptr(NULL) {

}

void nrg::Entity::markUpdated(){
	if(!nrg_updated) {
		nrg_updated = true;
		if(nrg_serv_ptr) {
			nrg_serv_ptr->markEntityUpdated(this);
		}
	}
}