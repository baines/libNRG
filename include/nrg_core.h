#ifndef NRG_CORE_H
#define NRG_CORE_H
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// adapted from http://gcc.gnu.org/wiki/Visibility
#if defined _WIN32 || defined __CYGWIN__
	#ifdef BUILDING_NRG
		#define NRG_LIB __declspec(dllexport)
	#else
		#define NRG_LIB __declspec(dllimport)
	#endif
#elif __GNUC__ >= 4 && defined BUILDING_NRG
	#define NRG_LIB __attribute__ ((visibility ("default")))
#else
	#define NRG_LIB
#endif

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
