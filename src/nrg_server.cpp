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
#include "nrg_server.h"
#include "nrg_config.h"
#include "nrg_player_impl.h"
#include "nrg_os.h"

using namespace std;
using namespace nrg;

namespace {
	typedef map<NetAddress, Player*> ClientMap;
	
	static void clearEntityUpdated(Entity* e){
		e->markUpdated(false);
	
		for(FieldBase* f = e->getFirstField(); f; f = f->getNextField()){
			f->setUpdated(false);
		}
	}
	
	static bool default_ver_func(uint32_t a, uint32_t b){
		return a == b;
	}
	
	static bool default_con_check(Server& s, Packet& p){
		const std::string& actual_name = s.getGameName();
		
		if(p.remaining() < PacketHeader::size + sizeof(Version) + actual_name.size() + sizeof(uint32_t)){
			return false;
		}
		
		p.seek(PacketHeader::size + sizeof(Version), SEEK_SET);
		std::string given_name;
		Codec<std::string>().decode(p, given_name);
		p.seek(0, SEEK_SET);
		
		if(given_name != actual_name){
			return false;
		} else {
			return true;
		}
	}
}

Server::Server(const std::string& game_name, uint32_t game_version, InputBase& input) 
: sock()
, buffer()
, input(&input)
, eventq()
, clients()
, global_msg_handlers()
, current_player(nullptr)
, timer(nrg::os::microseconds())
, game_name(game_name)
, game_version(game_version)
, max_players(0)
, version_func(&default_ver_func)
, connect_check(&default_con_check)
, user_pointer(nullptr)
, interval(NRG_DEFAULT_SERVER_INTERVAL_US){

}

Server::Server(const std::string& game_name, uint32_t game_version) 
: sock()
, buffer()
, input(nullptr)
, eventq()
, clients()
, global_msg_handlers()
, current_player(nullptr)
, timer(nrg::os::microseconds())
, game_name(game_name)
, game_version(game_version)
, max_players(0)
, version_func(&default_ver_func)
, connect_check(&default_con_check)
, user_pointer(nullptr)
, interval(NRG_DEFAULT_SERVER_INTERVAL_US){

}

bool Server::bind(const NetAddress& addr){
	sock.setFamilyFromAddress(addr);
	sock.setNonBlocking(true);
	sock.handleUnconnectedICMPErrors(true);
	
	return sock.bind(addr);
}

bool Server::isBound() {
	return sock.getBoundAddress() != nullptr;
}

static inline PlayerImpl* IMPL(nrg::Player* p){
	return static_cast<nrg::PlayerImpl*>(p);
}

bool Server::update(){
	eventq.clear();

	uint64_t blocktime;

	while(blocktime = max<int>(0, interval - (os::microseconds() - timer)),
	sock.dataPending(blocktime)){
		
		NetAddress addr;
		Status recv_status = sock.recvPacket(buffer.reset(), addr);
		
		ClientMap::iterator it = clients.find(addr);
		if(it == clients.end()){
			if(!recv_status) continue;
			
			// test if the packet is roughly of the right format before allocating a new player
			if(!connect_check(*this, buffer)) continue;
			
			printf("Client connecting: [%s:%d]\n", addr.getIP(), addr.getPort());
			
			uint16_t pid = player_ids.acquire();
			auto res = clients.insert(make_pair(addr, nullptr));
			it = res.first;
			
			it->second = new PlayerImpl(pid, *this, res.first->first);
			
			for(auto& m : global_msg_handlers){
				it->second->registerMessageHandler(*m);
			}
		}
				
		if(!recv_status){
			printf("Client quit: [%s:%d]\n", it->first.getIP(), it->first.getPort());
			
			player_ids.release(it->second->getID());
			PlayerEvent e = { PLAYER_LEAVE, it->second->getID(), it->second };
			eventq.pushEvent(e);
			
			delete it->second;
			clients.erase(it);
		} else {
			current_player = it->second;
			
			if(!IMPL(it->second)->addPacket(buffer)){
				it->second->kick("Recieved invalid packet from client.");
				printf("Client [%s:%d] was kicked (invalid packet).\n", addr.getIP(), addr.getPort());
			}
		}
	}
	
	timer = os::microseconds();

	// generate snapshot
	DeltaSnapshot& delta_ss = snaps.next();
	delta_ss.reset();
	delta_ss.setID(timer / 1000);
	master_snapshot.setID(timer / 1000);
	
	sort(updated_entities.begin(), updated_entities.end());
	
	for(uint16_t i : updated_entities){
		if(entities.size() > i){
			Entity* e = entities[i];
			
			if(e != nullptr){
				master_snapshot.addEntity(e);
				delta_ss.addEntity(e);
				clearEntityUpdated(e);
			} else {
				master_snapshot.removeEntityById(i);
				delta_ss.removeEntityById(i);
			}
		}
	}
	updated_entities.clear();

	for(ClientMap::iterator i = clients.begin(), j = clients.end(); i != j; /**/){
		if(!i->second->isConnected()){
			printf("Client quit: %s:%d\n", i->first.getIP(), i->first.getPort());
			
			player_ids.release(i->second->getID());
			PlayerEvent e = { PLAYER_LEAVE, i->second->getID(), i->second };
			eventq.pushEvent(e);

			delete i->second;
			clients.erase(i++);
		} else {
			++i;
		}
	}

	for(auto& c : clients){
		current_player = c.second;
		Status s;
		
		if(!(s = IMPL(c.second)->update())){
			c.second->kick("Client update failed.");
			const NetAddress& na = c.second->getRemoteAddress();
			const char* errmsg = s.desc;
			char errbuf[512];
			
			if(s.type == Status::SystemError){
				errmsg = strerr_r(s.sys_errno, errbuf, sizeof(errbuf));
			}
			
			printf("Client [%s:%d] was kicked. (%s)\n", na.getIP(), na.getPort(), errmsg);
		}
	}
	return true;
}

bool Server::pollEvent(Event& e){
	return eventq.pollEvent(e);
}

void Server::pushEvent(const Event& e){
	eventq.pushEvent(e);
}

size_t Server::playerCount() const {
	return clients.size();
}

void Server::setTickRate(uint8_t rate){
	interval = 1000000 / rate;
}

uint8_t Server::getTickRate() const {
	return 1000000 / interval;
}

void Server::registerEntity(Entity& e){
	e.setManager(this);
	uint16_t id = entity_ids.acquire();
	e.setID(id);
	if(entities.size() <= id){
		entities.resize(id+1);
	}
	entities[id] = &e;
	updated_entities.push_back(e.getID());
}

void Server::unregisterEntity(Entity& e){
	if(entities[e.getID()]){
		entity_ids.release(e.getID());
		updated_entities.push_back(e.getID());
		entities[e.getID()] = nullptr;
	}
}

void Server::markEntityUpdated(Entity& e){
	if(find(updated_entities.begin(), updated_entities.end(), e.getID()) 
	== updated_entities.end()){
		updated_entities.push_back(e.getID());
	}
}

void Server::broadcastMessage(const MessageBase& m){
	for(auto& c : clients){
		c.second->sendMessage(m);
	}
}

Player* Server::getPlayerByID(uint16_t id) const {
	auto i = find_if(clients.begin(), clients.end(), 
	[&](const pair<NetAddress, Player*>& p){
		return p.second->getID() == id;
	});
	return i == clients.end() ? nullptr : i->second;
}

bool Server::isGameVersionCompatible(uint32_t client_ver) const {
	return version_func(game_version, client_ver);
}

Server::~Server(){
	for(auto& c : clients){
		c.second->kick("Server closing.");
		delete c.second;
	}

	for(Entity* e : entities){
		if(e){
			e->setManager(nullptr);
			e->setID(0);
		}
	}
}
