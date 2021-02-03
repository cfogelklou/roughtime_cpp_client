/**
 * COPYRIGHT  (c)  Chris Fogelklou 2014
 * @file     endian_convert.h
 * @brief    Portable endian conversion routines to ease swapping between big-endian
 *           and little-endian on different machine types.
 */

#ifndef ENDIAN_CONVERT_H__
#define ENDIAN_CONVERT_H__

#include <cstdint>

#if !defined(NATIVE_LITTLE_ENDIAN) && !defined(NATIVE_BIG_ENDIAN)
#define NATIVE_LITTLE_ENDIAN
#endif

#define SWAPBYTES64(u64) ( \
((((uint64_t)(u64)) & 0x00000000000000ffull) << (64-(1*8))) | \
((((uint64_t)(u64)) & 0x000000000000ff00ull) << (64-(3*8))) | \
((((uint64_t)(u64)) & 0x0000000000ff0000ull) << (64-(5*8))) | \
((((uint64_t)(u64)) & 0x00000000ff000000ull) << (64-(7*8))) | \
((((uint64_t)(u64)) & 0x000000ff00000000ull) >> (64-(7*8))) | \
((((uint64_t)(u64)) & 0x0000ff0000000000ull) >> (64-(5*8))) | \
((((uint64_t)(u64)) & 0x00ff000000000000ull) >> (64-(3*8))) | \
((((uint64_t)(u64)) & 0xff00000000000000ull) >> (64-(1*8))) \
)
  
#define SWAPBYTES32(u32) ( \
((((uint32_t)(u32)) & 0x000000fful) << 24) | \
((((uint32_t)(u32)) & 0x0000ff00ul) << 8) | \
((((uint32_t)(u32)) & 0x00ff0000ul) >> 8) | \
((((uint32_t)(u32)) & 0xff000000ul) >> 24) \
)
  
#define SWAPBYTES16(u16) ( \
((((uint32_t)(u16)) & 0x00ff) << 8) | \
((((uint32_t)(u16)) & 0xff00) >> 8) \
)
  
#ifdef NATIVE_BIG_ENDIAN

#define HOSTTOLE64(u64) SWAPBYTES64(u64)
#define HOSTTOLE32(u32) SWAPBYTES32(u32)
#define HOSTTOLE16(u16) SWAPBYTES16(u16)
#define HOSTTOBE64(u64) (u64)
#define HOSTTOBE32(u32) (u32)
#define HOSTTOBE16(u16) (u16)
#define LE64TOHOST(u64) SWAPBYTES64(u64)
#define LE32TOHOST(u32) SWAPBYTES32(u32)
#define LE16TOHOST(u16) SWAPBYTES16(u16)
#define BE64TOHOST(u64) (u64)
#define BE32TOHOST(u32) (u32)
#define BE16TOHOST(u16) (u16)

#else // #ifdef NATIVE_BIG_ENDIAN
  
#define HOSTTOLE64(u64) (u64)
#define HOSTTOLE32(u32) (u32)
#define HOSTTOLE16(u16) (u16)
#define HOSTTOBE64(u64) SWAPBYTES64(u64)
#define HOSTTOBE32(u32) SWAPBYTES32(u32)
#define HOSTTOBE16(u16) SWAPBYTES16(u16)
#define LE64TOHOST(u64) (u64)
#define LE32TOHOST(u32) (u32)
#define LE16TOHOST(u16) (u16)
#define BE64TOHOST(u64) SWAPBYTES64(u64)
#define BE32TOHOST(u32) SWAPBYTES32(u32)
#define BE16TOHOST(u16) SWAPBYTES16(u16)
#endif
  
#endif // ENDIAN_CONVERT_H__
