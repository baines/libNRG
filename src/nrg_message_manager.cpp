#include "nrg_message_manager.h"
#include "nrg_os.h"
#include "nrg_util.h"
#include "nrg_varint.h"
#include <limits.h>
#include <algorithm>

using namespace std;
using namespace nrg;

namespace {
	template<class M, class F>
	static void removeMessagesUntil(Packet& buf, Packet& buf2, Packet*& msg_data
	, M& msg_metadata, const F& func){
		size_t bytes_to_remove = 0;

		auto it = find_if(msg_metadata.begin(), msg_metadata.end(), [&](const MessageManager::MessageInfo& m){
			bool b = func(m);
			if(!b) bytes_to_remove += m.data_len;
			return b;
		});
	
		msg_metadata.erase(msg_metadata.begin(), it);

		if(bytes_to_remove > 0){
			Packet* other_p = (msg_data == &buf) ? &buf2 : &buf;
		
			other_p->writeArray(msg_data->getBasePointer() + bytes_to_remove
				, msg_data->size() - bytes_to_remove
			);
		
			msg_data->reset();
			msg_data = other_p;
		
			for(auto& m : msg_metadata){
				m.data_offset -= bytes_to_remove;
			}
		}
	}
}

MessageManager::MessageManager()
: handlers()
, msg_metadata()
, msg_data(&buffer)
, buffer()
, buffer2()
, local_seq(0)
, remote_seq(0)
, up_to_date(true)
, waiting_for_response(false) {

}

void MessageManager::addHandler(MessageBase&& m){
	handlers.emplace(m.getID(), unique_ptr<MessageBase>(m.move_clone()));
}

void MessageManager::addHandler(const MessageBase& m){
	handlers.emplace(m.getID(), unique_ptr<MessageBase>(m.clone()));
}

void MessageManager::addMessage(const MessageBase& m, uint16_t server_ms){
	if(up_to_date){
		up_to_date = false;
		++local_seq;
	}

	MessageInfo i = { 
		msg_data->tell(),
		m.getID(),
		m.writeToPacket(*msg_data),
		local_seq,
		server_ms,
		static_cast<uint32_t>(os::milliseconds())
	};
	
	msg_metadata.push_back(i);
}

void MessageManager::writeToPacket(Packet& p, uint16_t server_ms){
	p.write8(remote_seq);
	
	uint32_t current_ms = os::milliseconds();
	
	// remove unacknowledged messages that are too old (> ~1 minute).
	removeMessagesUntil(buffer, buffer2, msg_data, msg_metadata, 
	[=](const MessageInfo& m){
		return (current_ms - m.local_creation_time_ms) < USHRT_MAX;
	});
			
	// stop here if there are no messages, or we haven't heard from the other side.
	if(waiting_for_response || msg_metadata.size() == 0) return;
	
	p.write8(local_seq);
	UVarint(msg_metadata.size()).encode(p);
	
	bool first_msg = true;
	
	// write all the stored messages.
	for(const MessageInfo& m : msg_metadata){
		if(first_msg){
			p.write16(m.server_creation_time_ms);
			first_msg = false;
		} else {
			UVarint(server_ms - m.server_creation_time_ms).encode(p);
		}
		UVarint(m.type_id).encode(p);
		p.writeArray(msg_data->getBasePointer() + m.data_offset, m.data_len);
	}
	
	up_to_date = true;
	waiting_for_response = true;
}

bool MessageManager::readFromPacket(Packet& p, uint16_t server_ms){
	if(p.remaining() < sizeof(uint8_t)) return false;
	
	uint8_t their_remote_seq = 0;
	p.read8(their_remote_seq);
	
	// remove acknowledged messages, so they're not sent any more.
	removeMessagesUntil(buffer, buffer2, msg_data, msg_metadata, 
	[&](const MessageInfo& m){
		return (their_remote_seq + 1) == m.creation_seq;
	});
	
	waiting_for_response = false;
	
	// if there are no messages the packet can end here, which isn't an error.
	if(!p.remaining()) return true;
		
	p.read8(remote_seq);
	uint32_t num_msgs = UVarint().quickDecode(p);
	
	uint32_t current_ms = os::milliseconds();
	
	// read all the messages recieved.
	for(uint32_t i = 0; i < num_msgs; ++i){
		uint16_t msg_ms = 0;
		
		if(i == 0){
			if(p.remaining() > sizeof(msg_ms)){
				p.read16(msg_ms);
			} else {
				return false;
			}
		} else {
			UVarint v;
			if(v.decode(p) == 0) return false;
			msg_ms = v.get();
		}
		uint16_t msg_diff_ms = server_ms - msg_ms;
		
		uint16_t id = UVarint().quickDecode(p);
		auto h_it = handlers.find(id);
		
		if(h_it == handlers.end()){
			// this message type hasn't been registered, that's no good.
			return false;
		} else {
			// read the message and invoke the user callback.
			if(h_it->second->readFromPacket(p) == 0) return false;
			h_it->second->onRecieve(current_ms - msg_diff_ms);
		}
	}
	
	return true;
}



