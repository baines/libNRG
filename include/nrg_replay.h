#ifndef NRG_REPLAY_H
#define NRG_REPLAY_H
#include "nrg_core.h"
#include "nrg_entity.h"
#include <zlib.h>

namespace nrg {

class ReplayRecorder {
public:
	ReplayRecorder();
	bool startRecording(const char* filename, int sid, const std::vector<Entity*>& initial_entities);
	bool isRecording() const;
	void stopRecording();

	void addPacket(Packet& p);
private:
	gzFile file;
	int status;
};

}

#endif
