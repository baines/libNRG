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
#ifndef NRG_REPLAY_SERVER_H
#define NRG_REPLAY_SERVER_H
#include "nrg_core.h"
#include "nrg_socket.h"
#include "nrg_server_state.h"
#include <zlib.h>

namespace nrg {

class  ReplayServer {
public:
	ReplayServer();
	bool openReplay(const char* filename);
	bool bind(const char* port);
	bool update();
	bool isReplayRunning() const;
private:
	NetAddress bind_addr, client_addr;
	UDPSocket sock;
	Connection con;
	ServerHandshakeState handshake;
	gzFile file;
	Packet buffer;
	uint32_t local_timer, remote_timer;
	int time_diff;
	bool started;
};

}

#endif
