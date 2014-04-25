#ifndef NRG_CORE_H
#define NRG_CORE_H
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "nrg_status.h"

// adapted from http://gcc.gnu.org/wiki/Visibility
#if defined _WIN32 || defined __CYGWIN__
	#ifdef BUILDING_NRG
		#define NRG_PUBLIC __declspec(dllexport)
	#else
		#define NRG_PUBLIC __declspec(dllimport)
	#endif
	#define NRG_PRIVATE
#elif __GNUC__ >= 4
	#define NRG_PUBLIC  __attribute__ ((visibility ("default")))
	#define NRG_PRIVATE __attribute__ ((visibility ("hidden")))
#else
	#define NRG_PUBLIC
	#define NRG_PRIVATE
#endif

#endif
