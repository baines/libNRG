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
#include "nrg_client.h"
#include "nrg_config.h"
#include "nrg_os.h"

using namespace nrg;

Client::Client(const char* game_name, uint32_t game_version, InputBase& input) 
: sock()
, input(&input)
, buffer()
, serv_addr()
, con(serv_addr, sock)
, state_con(con.out)
, eventq()
, state_manager(this)
, handshake()
, game_state()
, rate_limit_interval_ms(10)
, previous_ms(os::milliseconds())
, user_pointer(nullptr)
, dc_reason() {
	state_manager.addState(game_state);
	state_manager.addState(handshake);
}

Client::Client(const char* game_name, uint32_t game_version) 
: sock()
, input(nullptr)
, buffer()
, serv_addr()
, con(serv_addr, sock)
, state_con(con.out)
, eventq()
, state_manager(this)
, handshake()
, game_state()
, rate_limit_interval_ms(10)
, previous_ms(os::milliseconds())
, user_pointer(nullptr)
, dc_reason() {
	state_manager.addState(game_state);
	state_manager.addState(handshake);
}

bool Client::connect(const NetAddress& addr){
	sock.setFamilyFromAddress(addr);
	sock.setNonBlocking(true);
	sock.enableTimestamps(true);
#ifdef NRG_USE_SO_TIMESTAMP
	sock.setOption(SOL_SOCKET, SO_TIMESTAMP, 1);
#endif	
	serv_addr = addr;
	
	return sock.connect(serv_addr);
}

bool Client::update(){
	if(!isConnected()) return false;
	
	eventq.clear();

	while(sock.dataPending()){
		NetAddress addr;
		Status recv_status = sock.recvPacket(buffer.reset(), addr);
		
		if(addr.isValid() && addr != serv_addr) continue;
		
		if(!recv_status){
			const char* msg = recv_status.desc;
			
			if(recv_status.type == Status::SystemError){
				msg = strerr_r(recv_status.sys_errno, dc_reason, sizeof(dc_reason));
			}
			
			printf("Socket::recvPacket returned error %d: %s.\n", recv_status.sys_errno, msg);
			
			eventq.pushEvent(DisconnectEvent{ DISCONNECTED, msg });
			sock.disconnect();
			return false;
		}
		
		if(con.in.addPacket(buffer) && con.in.hasNewPacket()){
			PacketFlags f = con.in.getLatestPacket(buffer.reset());

			if(f & PKTFLAG_FINISHED){
				size_t sz = std::min(sizeof(dc_reason)-1, buffer.remaining());
				buffer.readArray((uint8_t*)dc_reason, sz);
				eventq.pushEvent(DisconnectEvent{ DISCONNECTED, dc_reason });
				sock.disconnect();
				return false;
			} else {
				if(!state_manager.onRecvPacket(buffer, f)){
					puts("RecvPacket failed!");
					return false;
				}
			}
		}
	}
	
	uint32_t current_ms = os::milliseconds();
	state_con.reset(current_ms - previous_ms > rate_limit_interval_ms);

	bool result = state_manager.update(state_con);
	
	if(state_con.sentPackets()){
		previous_ms = current_ms;
	}
		
	if(!result){
		puts("State Update failed!");
	}
	
	return result;
}

void Client::setPacketRateLimit(uint32_t packets_per_sec){
	rate_limit_interval_ms = 1 + 999 / std::max(1u, packets_per_sec);
}

bool Client::isConnected() const {
	return sock.isConnected();
}

void Client::registerEntity(Entity* e){
	game_state.registerEntity(e);
}

void Client::sendMessage(const MessageBase& m){
	game_state.sendMessage(m);
}

bool Client::pollEvent(Event& e){
	return eventq.pollEvent(e);
}

const ClientStats& Client::getStats() const {
	return game_state.getClientStats();
}

void Client::startRecordingReplay(const char* filename){
	game_state.startRecordingReplay(filename);
}

void Client::stopRecordingReplay(){
	game_state.stopRecordingReplay();
}

Client::~Client(){
	con.out.sendDisconnect(buffer.reset());
}
