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
#ifndef NRG_MESSAGE_H
#define NRG_MESSAGE_H
#include "nrg_core.h"
#include "nrg_packet.h"
#include "nrg_codec.h"
#include <tuple>
#include <functional>

namespace nrg {

struct MessageBase {
	virtual uint16_t getID() const = 0;
	virtual size_t writeToPacket(Packet& p) const = 0;
	virtual size_t readFromPacket(Packet& p) = 0;
	virtual void onRecieve(uint32_t creation_ms) = 0;
	virtual MessageBase* clone() const = 0;
	virtual MessageBase* move_clone() = 0;
	virtual ~MessageBase(){};
};

using std::enable_if;
using std::tuple_element;

template<uint16_t id, typename... Args>
class Message : public MessageBase {
	static const size_t sz = sizeof...(Args)-1;
	typedef std::tuple<Args...> MsgTuple;
public:
	Message(Args... args) : values(args...), on_recieve(){}
	Message(Message&&) = default;
	Message(const Message&) = default;
	
	template<class F>
	Message(F&& func)
	: values()
	, on_recieve(std::forward<F>(func)){}
		
	template<size_t n>
	typename std::enable_if<n == sz, size_t>::type do_write(Packet& p) const {
		return Codec<typename tuple_element<n, MsgTuple>::type>().encode(p, std::get<n>(values));
	}
	
	template<size_t n>
	typename std::enable_if<n != sz, size_t>::type do_write(Packet& p) const {
		return Codec<typename tuple_element<n, MsgTuple>::type>().encode(p, std::get<n>(values))
		+ do_write<n+1>(p);
	}
	
	template<size_t n>
	typename std::enable_if<n == sz, size_t>::type do_read(Packet& p){
		return Codec<typename tuple_element<n, MsgTuple>::type>().decode(p, std::get<n>(values));
	}
	
	template<size_t n>
	typename std::enable_if<n != sz, size_t>::type do_read(Packet& p){
		return Codec<typename tuple_element<n, MsgTuple>::type>().decode(p, std::get<n>(values))
		+ do_read<n+1>(p);
	}
		
	uint16_t getID() const {
		return id;
	}
	
	void onRecieve(uint32_t creation_ms){
		if(on_recieve) on_recieve(*this, creation_ms);
	}
	
	MessageBase* clone() const {
		return new Message(*this);
	}
	
	MessageBase* move_clone() {
		return new Message(std::move(*this));
	}
	
	size_t writeToPacket(Packet& p) const {
		return do_write<0>(p);
	}
	
	size_t readFromPacket(Packet& p){
		return do_read<0>(p);
	}
	
	template<size_t n>
	const typename tuple_element<n, MsgTuple>::type& get() const {
		return std::get<n>(values);
	}
	
	template<size_t n>
	void set(const typename tuple_element<n, MsgTuple>::type& val) {
		std::get<n>(values) = val;
	}
	
private:
	MsgTuple values;
	std::function<void(const Message&, uint32_t)> on_recieve;
};

}
#endif
