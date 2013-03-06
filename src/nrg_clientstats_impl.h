#ifndef NRG_CLIENTSTATS_IMPL_H
#define NRG_CLIENTSTATS_IMPL_H
#include "nrg_core.h"
#include "nrg_util.h"

namespace nrg {

struct ClientStatsImpl : public ClientStats {
	ClientStatsImpl() : s_stats(), i_stats(), s_index(0), i_index(0){}

	size_t getNumSnapshotStats() const;
	int getSnapshotStat(size_t index) const;

	size_t getNumInterpStats() const;
	int getInterpStat(size_t index) const;
	
	void addSnapshotStat(int stat);
	void addInterpStat(int stat);

	uint8_t* toRGBATexture(uint32_t (&tex)[64*64]) const;
private:
	static const int NUM_STATS = 64;
	int s_stats[NUM_STATS];
	int i_stats[NUM_STATS];
	int s_index, i_index;
};

}

#endif
