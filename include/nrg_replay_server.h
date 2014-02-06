#ifndef NRG_REPLAY_SERVER_H
#define NRG_REPLAY_SERVER_H
#include "nrg_core.h"
#include "nrg_socket.h"
#include "nrg_server_state.h"
#include <zlib.h>

namespace nrg {

class NRG_LIB ReplayServer {
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
