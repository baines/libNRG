#ifndef NRG_CONFIG_H
#define NRG_CONFIG_H

static const size_t NRG_MAX_PACKET_SIZE = 1400;
static const size_t NRG_NUM_PAST_SNAPSHOTS = 32;
static const size_t NRG_CONN_PACKET_HISTORY = NRG_NUM_PAST_SNAPSHOTS;
static const size_t NRG_MAX_ERRORMSG_LEN = 256;

#define NRG_ENABLE_ZLIB_COMPRESSION 1
#define NRG_USE_SO_TIMESTAMP 1


#endif
