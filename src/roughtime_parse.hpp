#ifndef ROUGHTIME_PARSE_HPP
#define ROUGHTIME_PARSE_HPP 1

/**
* COPYRIGHT	(c)	Applicaudia 2019
* @file     roughtime_parse.hpp
* @brief    Parses a roughtime response in firmware or app.
*/



#ifdef __cplusplus
#include "roughtime_request.hpp"
#include <string>
#include <cstdint>
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
