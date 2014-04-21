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
	void removeMessagesUntil(const std::function<bool(const MessageInfo&)>&);
	std::map<uint16_t, std::unique_ptr<MessageBase>> handlers;
	std::vector<MessageInfo> msg_metadata;
	Packet *msg_data, buffer, buffer2;
	uint8_t local_seq, remote_seq;
	bool up_to_date, waiting_for_response;
};

}

#endif
