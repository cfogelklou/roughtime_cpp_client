#pragma once

#include <cstdint>
#include <string>
#include "endian_convert.h"

namespace std {
typedef basic_string<uint8_t, char_traits<uint8_t>, allocator<uint8_t> > ustring;
}

#define NONC_AS_U32 0x434e4f4e

class RtClient {
public:
  static const uint32_t nonc_le;
  
  // Do not use this struct directly; it is only here for documentation
  // purposes.
  typedef struct RtHeaderTag {
    uint32_t num_tags_le;
    uint32_t offets[1]; // offsets[MAX(0, num_tags-1)]
    uint32_t tags[1];   // tags[num_tags]
  } RtHeaderT;
  
public:
  RtClient();
  ~RtClient();
  
  void GenerateRequest(std::ustring &request);
  
  static void PadRequest(const std::ustring &unpadded, std::ustring &padded);
  
private:
  uint8_t nonce[64];
  uint64_t ts_request;
};
