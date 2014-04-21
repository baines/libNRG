#include "nrg_client.h"
#include "nrg_config.h"

using namespace nrg;

Client::Client(const char* game_name, uint32_t game_version, InputBase& input) 
: sock()
, input(&input)
, buffer()
, serv_addr()
, con(serv_addr, sock)
, eventq()
, state_manager(this)
, handshake()
, game_state()
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
, eventq()
, state_manager(this)
, handshake()
, game_state()
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
		sock.recvPacket(buffer.reset(), addr);
		if(addr != serv_addr) continue;
		if(con.in.addPacket(buffer) && con.in.hasNewPacket()){
			PacketFlags f = con.in.getLatestPacket(buffer.reset());

			if(f & PKTFLAG_FINISHED){
				size_t sz = std::min(sizeof(dc_reason)-1, buffer.remaining());
				buffer.readArray((uint8_t*)dc_reason, sz);
				
				DisconnectEvent de = { DISCONNECTED, dc_reason };
				eventq.pushEvent(de);
				
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
	
	bool b = state_manager.update(con.out);
	if(!b) puts("State Update failed!");
	return b;
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
