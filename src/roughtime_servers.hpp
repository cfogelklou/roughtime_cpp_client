#ifndef ROUGHTIME_PAK_SERVERS_HPP
#define ROUGHTIME_PAK_SERVERS_HPP

/**
* COPYRIGHT	(c)	Polestar 2019
* @file     roughtime_servers.hpp
* @brief    Indexable list of roughtime servers trusted by the firmware.
*/

#include <stdint.h>
#include <stdarg.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

// ////////////////////////////////////////////////////////////////////////////
typedef struct RoughtimeServerTag {
  const char * const  addr;
  const int           port;
  const char * const  keyHex;
} RoughtimeServer;

extern const RoughtimeServer RoughtimeServersArr[];
extern const size_t RoughtimeServersCount;

// ////////////////////////////////////////////////////////////////////////////
// Returns name of the server if it is trusted, otherwise NULL.
const char * RoughtimeServerIsTrusted(const uint8_t keyBin[], const size_t keySz);

// ////////////////////////////////////////////////////////////////////////////
const RoughtimeServer * RoughtimeGetServerAtIdx(const int idx);

// ////////////////////////////////////////////////////////////////////////////
bool RoughtimeGetKey(const RoughtimeServer *, uint8_t keyBin[32]);

// ////////////////////////////////////////////////////////////////////////////
bool RoughtimeGetKeyAtIdx(const int idx, uint8_t keyBin[32]);

#ifdef __cplusplus
}
#endif


#endif
