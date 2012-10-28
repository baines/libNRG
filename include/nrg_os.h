#ifndef NRG_OS_H
#define NRG_OS_H

#if defined __linux
	#include "os/nrg_linux.h"
#elif defined __WIN32
	#include "os/nrg_windows.h"
#endif

#endif
