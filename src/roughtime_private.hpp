#ifndef ROUGHTIME_PRIVATE_HPP
#define ROUGHTIME_PRIVATE_HPP

#ifdef __cplusplus

#include <assert.h>
#include <cstdio>
#include "endian_convert.h"
#define u_str() data()
#define MIN(x,y) (((x) < (y)) ? (x) : (y))
#define LOG_WARNING(x) printf x
#define LOG_ASSERT assert
#define LOG_ASSERT_WARN(x) if (!(x)) do { printf("Warning at %s (%d)\r\n", __FILE__, __LINE__);} while(0)
#endif

#endif
