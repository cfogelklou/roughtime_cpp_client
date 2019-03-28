#ifndef ROUGHTIME_PARSE_HPP
#define ROUGHTIME_PARSE_HPP 1

#include <string>
#include <cstdint>

#ifdef __cplusplus

#include "roughtime_common.hpp"

namespace RoughTime {
  

  typedef struct ParseOutTag {
    uint64_t midpoint;
    uint64_t mintime;
    uint64_t maxtime;
    uint32_t radius;
  } ParseOutT;

  uint64_t ParseToMicroseconds(
    const uint8_t pubkey[32],
    const uint8_t nonce[64],
    const uint8_t b[],
    const size_t b_length,
    ParseOutT *const pOut = nullptr
  );
  
};

#endif // #ifdef __cplusplus
#endif
