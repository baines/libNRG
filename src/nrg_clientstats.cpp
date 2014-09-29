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
float ClientStatsImpl::getInterpStat(size_t index) const {
	return i_stats[(i_index + 1 + index) % NUM_STATS];
}

void ClientStatsImpl::addSnapshotStat(int stat) {
	s_index = (s_index + 1) % NUM_STATS;
	s_stats[s_index] = stat;
}
void ClientStatsImpl::addInterpStat(float stat){
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

	int height = NUM_STATS / 2;
	int width  = NUM_STATS;

	// upper interpolation graph
	for(int h = 0; h < height; ++h){
		for(int w = 0; w < width; ++w){
			uint32_t& p = tex[h*width+w];
			float stat = getInterpStat(w);

			if(stat * 4.0f >= (height-h)){
				p = stat <= 1.0f ? BLU : YLW;
			} else { // background
				int x = (i_index + w) % width;
				p = x >= width / 2 ? BG1 : BG2;
			}
		}
	}

	// lower snapshot ping graph
	for(int h = height; h < height * 2; ++h){
		for(int w = 0; w < width; ++w){
			uint32_t& p = tex[h*width+w];
			int stat = getSnapshotStat(w);

			if(stat < 0){ // dropped
				p = RED;
			} else if(1 + stat / 8 >= (height*2-h)){ // recieved
				p = GRN;
			} else { // background
				int x = (s_index + w) % width;
				p = x >= width / 2 ? BG1 : BG2;
			}
		}
	}
	return reinterpret_cast<uint8_t*>(tex);
}
