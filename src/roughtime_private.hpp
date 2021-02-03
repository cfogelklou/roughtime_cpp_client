#ifndef ROUGHTIME_PRIVATE_HPP
#define ROUGHTIME_PRIVATE_HPP

#ifdef __cplusplus
#include "endian_convert.h"

#include <assert.h>
#include <cstdio>
#include <string>
typedef std::basic_string<uint8_t, std::char_traits<uint8_t>, std::allocator<uint8_t> > sstring;

#define u_str() data()
#define MIN(x,y) (((x) < (y)) ? (x) : (y))
#define LOG_WARNING(x) printf x
#define LOG_ASSERT assert
#define LOG_ASSERT_WARN(x) if (!(x)) do { printf("Warning at %s (%d)\r\n", __FILE__, __LINE__);} while(0)
#define ARRSZ(arr) (sizeof((arr))/sizeof((arr)[0]))
#endif

#endif
