#include "nrg_entity.h"
#include "nrg_server.h"
#include "nrg_client_state.h"

nrg::Entity::Entity() : nrg_id(0), nrg_updated(false), 
nrg_serv_ptr(NULL), nrg_cgs_ptr(NULL) {

}

void nrg::Entity::markUpdated(){
	if(!nrg_updated) {
		nrg_updated = true;
		if(nrg_serv_ptr) {
			nrg_serv_ptr->markEntityUpdated(this);
		}
	}
}

nrg::Entity::~Entity(){
	if(nrg_serv_ptr){
		nrg_serv_ptr->unregisterEntity(this);
	}
}

double nrg::Entity::getClientSnapshotTiming() const {
	if(nrg_cgs_ptr){
		return nrg_cgs_ptr->getSnapshotTiming();
	} else {
		return 0.0;
	}
}
