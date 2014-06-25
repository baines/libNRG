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
#ifndef NRG_STATE_MANAGER_H
#define NRG_STATE_MANAGER_H
#include "nrg_core.h"
#include "nrg_packet.h"
#include "nrg_message.h"
#include <memory>
#include <vector>
#include <map>

namespace nrg {

class MessageManager {
public:
	MessageManager();
	void writeToPacket(Packet& p, uint16_t server_ms);
	bool readFromPacket(Packet& p, uint16_t server_ms);
	
	void addHandler(MessageBase&& m);
	void addHandler(const MessageBase& m);
	
	void addMessage(const MessageBase& m, uint16_t server_ms);
private:
	struct MessageInfo {
		off_t data_offset;
		uint16_t type_id;
		size_t data_len;
		uint8_t creation_seq;
		uint16_t server_creation_time_ms;
		uint32_t local_creation_time_ms;
	};
	std::map<uint16_t, std::unique_ptr<MessageBase>> handlers;
	std::vector<MessageInfo> msg_metadata;
	Packet *msg_data, buffer, buffer2;
	uint8_t local_seq, remote_seq;
	bool up_to_date, waiting_for_response;
};

}

#endif
