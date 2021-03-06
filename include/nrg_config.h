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
 *  Contains several definitions that can be used to configure the library at compile time
 */
#ifndef NRG_CONFIG_H
#define NRG_CONFIG_H

/** The maximum size of a single UDP datagram sent / received */ 
static const size_t NRG_MAX_PACKET_SIZE = 1400;

/** The number of snapshots-diffs that are stored by the server */
static const size_t NRG_NUM_PAST_SNAPSHOTS = 32;

/** The size of the window in which out-of-order packets are accepted */
static const size_t NRG_CONN_PACKET_HISTORY = NRG_NUM_PAST_SNAPSHOTS;

/** The maximum length of an error message sent over the network */
static const size_t NRG_MAX_ERRORMSG_LEN = 256;

/** The default tick frequency of the server in microseconds */
static const size_t NRG_DEFAULT_SERVER_INTERVAL_US = 50000;

/** The max number of delta-snapshots the server will send without an acknowledgement before waiting */
static const size_t NRG_MAX_SNAPS_WITHOUT_ACK = NRG_NUM_PAST_SNAPSHOTS / 2;

#ifdef __linux
    #define NRG_ENABLE_ZLIB_COMPRESSION 1
    #define NRG_USE_SO_TIMESTAMP 1
    #define NRG_ENABLE_MSG_ERRQUEUE 1
#endif

#endif
