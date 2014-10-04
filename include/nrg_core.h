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
/** @file
 *  Common defines and includes used by all the other nrg header files
 */
#ifndef NRG_CORE_H
#define NRG_CORE_H
#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#define _CRT_RAND_S
	#include <stdlib.h>
	#include "windows.h"
	#include "winsock2.h"
	#include "ws2tcpip.h"
	#include "mswsock.h"
	typedef int socklen_t;
#else
	#include <unistd.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <sys/time.h>
	#include <netdb.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
#endif
#include "stdint.h"
#include "nrg_status.h"

#ifndef INET6_ADDRSTRLEN
#define INET6_ADDRSTRLEN 46
#endif

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

#ifdef DEBUG
	#define NRG_DEBUG(fmt, ...) do { fprintf(stderr, fmt, __VA_ARGS__); } while (0)
#else
	#define NRG_DEBUG(fmt, ...) do { } while(0)
#endif

/** @mainpage
 *  \section intro Introduction
 *  \par
 *      This is the documentation for LibNRG (Networking for Real-time Games) - a library which was created by myself, Alex Baines, for my third year computer science project at Warwick University.
 *  \par
 *      The library provides a high-level of abstraction over UDP networking that can be used to implement client / server online multi-player in video games.
 *  \section src Source Code
 *  \par
 *      <a href="https://github.com/baines/LibNRG/">Available on Github here.</a>
 *  \section classes Main classes
 *  - nrg::Server
 *      - Create one server-side to set up a game server
 *  - nrg::Client
 *      - Create one client-side to set up a game client
 *  - nrg::Entity
 *      - Inherit from this class or EntityHelper and put some nrg::Field classes inside to define a struct that will be replicated from the server to all connected Clients
 *  - nrg::Input
 *      - Inherit a single class from this CRTP class and puts nrg::Field classes inside it, this will be the data the client sends to the server each frame such as mouse position, camera angles
 *  - nrg::Field
 *      - Wrap types like int in this template class to define a piece of data to be replicated inside an nrg::Entity or nrg::Input.
 *  - nrg::Message
 *      - Create instances of this variadic template class to define RPC methods between server and clients
 *  \section nav Links
 *  \par
 *  The other classes are less important (and may only be for internal use) but still have some associated documentation.
 *  - <a href="annotated.html">Class List</a>
 *  - <a href="files.html">File List</a>
 */

#endif
