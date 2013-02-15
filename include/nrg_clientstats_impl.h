#ifndef NRG_CLIENTSTATS_IMPL_H
#define NRG_CLIENTSTATS_IMPL_H
#include "nrg_util.h"
#include "nrg_endian.h"
namespace nrg {

struct ClientStatsImpl : public ClientStats {
	ClientStatsImpl() : s_stats(), i_stats(), s_index(0), i_index(0){}
	size_t getNumSnapshotStats() const {
		return NUM_STATS;
	}
	int getSnapshotStat(size_t index) const {
		return s_stats[(s_index + (NUM_STATS-1) + index) % NUM_STATS];
	}

	size_t getNumInterpStats() const {
		return NUM_STATS;
	}
	int getInterpStat(size_t index) const {
		return i_stats[(i_index + (NUM_STATS-1) + index) % NUM_STATS];
	}
	
	void addSnapshotStat(int stat) {
		s_stats[++s_index % NUM_STATS] = stat;
	}
	void addInterpStat(int stat){
		i_stats[++i_index % NUM_STATS] = stat;
	}

	void toRGBATexture(uint32_t (&tex)[32*32]) const {
		memset(&tex, 0, sizeof(tex));
		
		for(int h = 0; h < 16; ++h){
			for(int w = 0; w < 32; ++w){
				if(getInterpStat(w) >= (16-h)){
					tex[h*32+w] = h == 15 ? ntoh(0x0000ffff) : ntoh(0xffff00ff);
				}
			}
		}
		for(int h = 16; h < 32; ++h){
			for(int w = 0; w < 32; ++w){
				if(getSnapshotStat(w) == 0) tex[h*32+w] = ntoh(0xff0000ff);
				else if(getSnapshotStat(w) >= (32-h)) tex[h*32+w] = ntoh(0x00ff00ff);
			}
		}
	}

private:
	static const int NUM_STATS = 32;
	static const uint32_t RED = 0xff0000ff;
	static const uint32_t GRN = 0x00ff00ff;
	static const uint32_t BLU = 0x0000ffff;
	static const uint32_t YLW = 0x00ffffff;

	int s_stats[NUM_STATS];
	int i_stats[NUM_STATS];

	int s_index, i_index;
};

}

#endif
