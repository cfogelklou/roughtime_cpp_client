#ifndef ROUGHTIME_PRIVATE_HPP
#define ROUGHTIME_PRIVATE_HPP

#ifdef __cplusplus
#ifdef __POLESTAR_PAK
#include "utils/ble_log.h"
#include "utils/helper_macros.h"
#include "platform/endian_convert.h"

LOG_MODNAME("roughtime.cpp");
#else
#include <assert.h>
#include "endian_convert.h"
#define u_str() data()
#define MIN(x,y) (((x) < (y)) ? (x) : (y))
#define LOG_WARNING(x) printf x
#define LOG_ASSERT assert
#define LOG_ASSERT_WARN(x) if (!(x)) do { printf("Warning at %s (%d)\r\n", __FILE__, __LINE__);} while(0)
#endif

#endif
#endif
