#include "nrg_clientstats_impl.h"
#include "nrg_endian.h"

using namespace nrg;

size_t ClientStatsImpl::getNumSnapshotStats() const {
	return NUM_STATS;
}
int ClientStatsImpl::getSnapshotStat(size_t index) const {
	return s_stats[(s_index + 1 + index) % NUM_STATS];
}

size_t ClientStatsImpl::getNumInterpStats() const {
	return NUM_STATS;
}
int ClientStatsImpl::getInterpStat(size_t index) const {
	return i_stats[(i_index + 1 + index) % NUM_STATS];
}

void ClientStatsImpl::addSnapshotStat(int stat) {
	s_index = (s_index + 1) % NUM_STATS;
	s_stats[s_index] = stat;
}
void ClientStatsImpl::addInterpStat(int stat){
	i_index = (i_index + 1) % NUM_STATS;
	i_stats[i_index] = stat;
}

static const uint32_t RED = ntoh(0xff0000ff);
static const uint32_t GRN = ntoh(0x00ff00ff);
static const uint32_t BLU = ntoh(0x0000ffff);
static const uint32_t YLW = ntoh(0xffff00ff);

static const uint32_t BG1 = ntoh(0x111111ff);
static const uint32_t BG2 = ntoh(0x222222ff);

uint8_t* ClientStatsImpl::toRGBATexture(uint32_t (&tex)[NUM_STATS*NUM_STATS]) const {		
	for(int h = 0; h < NUM_STATS/2; ++h){
		for(int w = 0; w < NUM_STATS; ++w){
			uint32_t& p = tex[h*NUM_STATS+w];
			if(getInterpStat(w) >= ((NUM_STATS/2)-h)){
				p = h == (NUM_STATS/2-1) ? BLU : YLW;
			} else {
				int x = (i_index + w) % NUM_STATS;
				p = x >= (NUM_STATS/2) ? BG1 : BG2;
			}
		}
	}
	for(int h = NUM_STATS/2; h < NUM_STATS; ++h){
		for(int w = 0; w < NUM_STATS; ++w){
			uint32_t& p = tex[h*NUM_STATS+w];
			if(getSnapshotStat(w) < 0){
				p = RED;
			} else if(1 + getSnapshotStat(w) / 8 >= (NUM_STATS-h)){
				p = GRN;
			} else {
				int x = (s_index + w) % NUM_STATS;
				p = x >= (NUM_STATS/2) ? BG1 : BG2;
			}
		}
	}
	return reinterpret_cast<uint8_t*>(tex);
}
