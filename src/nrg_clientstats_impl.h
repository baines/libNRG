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
