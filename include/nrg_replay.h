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
/** @file
 *  Functionality for recording replay files
 */
#ifndef NRG_REPLAY_H
#define NRG_REPLAY_H
#include "nrg_core.h"
#include "nrg_entity.h"

#ifdef NRG_ENABLE_ZLIB_COMPRESSION
#include <zlib.h>
#else
typedef void* gzFile;
#endif

namespace nrg {

/** Class that writes a replay to a file */
class ReplayRecorder {
public:
	/** Default constructor */
	ReplayRecorder();

	/** Starts recording a replay !CURRENTLY BROKEN! */
	bool startRecording(const char* filename, int sid, const std::vector<Entity*>& initial_entities);

	/** Returns true if a replay is being recorded */
	bool isRecording() const;

	/** Stops recording a replay */
	void stopRecording();

	/** Adds a packet that will be serialised into the replay file */
	void addPacket(Packet& p);
private:
	gzFile file;
	int status;
};

}

#endif
