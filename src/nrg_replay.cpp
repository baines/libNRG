#include "nrg_replay.h"
#include "nrg_replay_server.h"
#include "nrg_snapshot.h"
#include "nrg_config.h"
#include "nrg_os.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace nrg;

static const int STATUS_ERROR = -1;
static const int STATUS_STOPPED = 0;
static const int STATUS_RECORDING = 2;
static const char header[] = "NRG Replay File";

ReplayRecorder::ReplayRecorder() : file(NULL), status(STATUS_STOPPED){

}

typedef std::vector<Entity*>::const_iterator E_cit;

bool ReplayRecorder::startRecording(const char* filename, int sid, const std::vector<Entity*>& ents){
	if(isRecording()) stopRecording();

	int fd = open(filename, O_WRONLY | O_CREAT, 00640);
	if(fd < 0) return false;

	write(fd, header, sizeof(header));

	file = gzdopen(fd, "wb");
	if(!file) return false;

	if(sid != -1){
		Snapshot ss;
		Packet p;
		for(E_cit i = ents.begin(), j = ents.end(); i != j; ++i){
			if(*i) ss.addEntity(*i);
		}
	
		p.write16(sid).write32(os::microseconds()/1000).write16(0).write16(0);
		ss.writeToPacket(p);
		addPacket(p);
	}

	status = STATUS_RECORDING;

	return true;
}

bool ReplayRecorder::isRecording() const {
	return status == STATUS_RECORDING;
}

void ReplayRecorder::stopRecording() {
	gzclose(file);
	status = STATUS_STOPPED;
}

void ReplayRecorder::addPacket(Packet& p){
	uint32_t sz = p.size();
	gzwrite(file, &sz, sizeof(sz));
	gzwrite(file, p.getBasePointer(), sz);
}

ReplayServer::ReplayServer() : bind_addr(), client_addr(), 
sock(), in(client_addr), out(client_addr, sock), handshake(), file(), buffer(),
local_timer(), remote_timer(), time_diff(), started(false) {

}

bool ReplayServer::openReplay(const char* filename){
	int fd = open(filename, O_RDONLY);

	char buff[sizeof(header)];
	read(fd, buff, sizeof(header));
	buff[sizeof(header)-1] = '\0';

	if(memcmp(buff, header, sizeof(header)-1) != 0) return false;

	file = gzdopen(fd, "rb");
	return file != NULL;
}

bool ReplayServer::bind(const char* port){
	bind_addr.resolve("127.0.0.1", port);
	return sock.bind(bind_addr);
}

bool ReplayServer::update(){
	while(sock.dataPending()){
		NetAddress addr;
		sock.recvPacket(buffer.reset(), addr);

		if(in.addPacket(buffer) && in.hasNewPacket()){
			in.getLatestPacket(buffer.reset());
			
			if(in.isLatestPacketFinal()){
				return false;
			} else if(!client_addr.isValid()){
				client_addr = addr;
				handshake.addIncomingPacket(buffer);
			}
		}
	}

	local_timer = os::microseconds() / 1000;

	if(handshake.needsUpdate()){
		handshake.update(out);
	} else if(client_addr.isValid()){
		if(!started){
			gzseek(file, 6, SEEK_CUR);
			gzread(file, &remote_timer, 4);
			remote_timer = ntoh(remote_timer);
			gzseek(file, -10, SEEK_CUR);
			time_diff = local_timer - remote_timer;
			started = true;
		}

		if(local_timer - time_diff >= remote_timer){
			uint8_t buff[NRG_MAX_PACKET_SIZE];

			uint32_t len = 0;
			gzread(file, &len, sizeof(len));
			len = std::min<int>(len, NRG_MAX_PACKET_SIZE);

			gzread(file, buff, len);

			out.sendPacket(buffer.reset().writeArray(buff, len));
		
			if(!gzeof(file)){
				gzseek(file, 6, SEEK_CUR);
				gzread(file, &remote_timer, 4);
				remote_timer = ntoh(remote_timer);
				gzseek(file, -10, SEEK_CUR);
			}
		}
	}
	return true;
}

