#ifndef NRG_MESSAGE_H
#define NRG_MESSAGE_H
#include "nrg_core.h"
#include "nrg_packet.h"
#include <tuple>
#include <functional>

namespace nrg {

struct MessageBase {
	virtual int getID() const = 0;
	virtual size_t writeToPacket(Packet& p) const = 0;
	virtual size_t readFromPacket(Packet& p) = 0;
	virtual void onRecieve() = 0;
	virtual MessageBase* clone() const = 0;
	virtual ~MessageBase(){}
};

using std::enable_if;
using std::tuple_element;

template<int id, typename... Args>
class Message : public MessageBase {
	static const size_t sz = sizeof...(Args)-1;
	typedef std::tuple<Args...> MsgTuple;
public:
	Message(Args... args) : values(args...), on_recieve(){}
	Message(const std::function<void(const Message&)>& func) : values(), on_recieve(func){}
		
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
		
	int getID() const {
		return id;
	}
	
	void onRecieve(){
		if(on_recieve) on_recieve(*this);
	}
	
	MessageBase* clone() const {
		return new Message(*this);
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
	std::function<void(const Message&)> on_recieve;
};

}
#endif
