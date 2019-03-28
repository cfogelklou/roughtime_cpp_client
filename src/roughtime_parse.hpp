#ifndef ROUGHTIME_HPP
#define ROUGHTIME_HPP 1


#include <string>
#include <cstdint>


#ifdef __cplusplus

#include "roughtime_common.hpp"

class RtParse {
public:
  
  // Do not use this struct directly; it is only here for documentation
  // purposes.
  typedef struct RtHeaderTag {
    uint32_t num_tags_le;
    uint32_t offets[1]; // offsets[MAX(0, num_tags-1)]
    uint32_t tags[1];   // tags[num_tags]
  } RtHeaderT;
  
public:

  typedef void (*GenRandomFn)(void *p, uint8_t *buf, const size_t buflen);

  RtParse();
  ~RtParse();

  typedef struct ParseOutTag {
    uint64_t midpoint;
    uint64_t mintime;
    uint64_t maxtime;
    uint32_t radius;
  } ParseOutT;

  static uint64_t Parse(
    const uint8_t pubkey[32],
    const uint8_t nonce[64],
    const uint8_t b[],
    const size_t b_length,
    ParseOutT *const pOut = nullptr
  );
  
};

#endif // #ifdef __cplusplus
#endif
