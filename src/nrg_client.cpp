#include "nrg_client.h"
#include "nrg_config.h"

using namespace nrg;

Client::Client(const NetAddress& addr, Input& input) 
: sock(addr)
, input(input)
, buffer()
, serv_addr(addr)
, con(serv_addr, sock)
, eventq()
, state_manager(this)
, handshake()
, game_state(eventq, sock, input), dc_reason() {
	sock.setNonBlocking(true);
	sock.enableTimestamps(true);
#ifdef NRG_USE_SO_TIMESTAMP
	sock.setOption(SOL_SOCKET, SO_TIMESTAMP, 1);
#endif
	state_manager.addState(game_state);
	state_manager.addState(handshake);
}

bool Client::update(){
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

				return true;
			} else {
				if(!state_manager.onRecvPacket(buffer, f)){
					return false;
				}
			}
		}
	}
	
	return state_manager.update(con.out);
}

void Client::registerEntity(Entity* e){
	game_state.registerEntity(e);
}

void Client::registerMessage(const MessageBase& m){
	game_state.registerMessage(m);
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
