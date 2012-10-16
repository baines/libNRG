#ifndef NRG_CORE_H
#define NRG_CORE_H
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace nrg {
namespace status {
	enum status_t {
		OK    =  0,
		ERROR = -1,
		NYI   = -2
	};
};

typedef enum nrg::status::status_t status_t;

};

#endif
