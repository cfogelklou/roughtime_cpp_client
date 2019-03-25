#ifndef ROUGHTIME_HPP
#define ROUGHTIME_HPP 1



#include "platform/endian_convert.h"
#include "utils/simple_string.hpp"
#include <cstring>
#include <cstdint>

#ifdef __cplusplus

namespace roughtime {
  // These must be LE-ified
  static const uint32_t CERT = 0x54524543;
  static const uint32_t DELE = 0x454c4544;
  static const uint32_t INDX = 0x58444e49;
  static const uint32_t MAXT = 0x5458414d;
  static const uint32_t MIDP = 0x5044494d;
  static const uint32_t MINT = 0x544e494d;
  static const uint32_t NONC = 0x434e4f4e;
  static const uint32_t PAD = 0xff444150;
  static const uint32_t PATH = 0x48544150;
  static const uint32_t PUBK = 0x4b425550;
  static const uint32_t RADI = 0x49444152;
  static const uint32_t ROOT = 0x544f4f52;
  static const uint32_t SIG = 0x00474953;
  static const uint32_t SREP = 0x50455253;
}

class RtClient {
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

  RtClient(
    GenRandomFn genRandom = nullptr,
    void *genRandomData = nullptr
  );
  ~RtClient();
  
  void GenerateRequest(
    sstring &request,
    const uint8_t *nonce = nullptr,
    const size_t  nonceLen = 0
  );

  static void PadRequest(const sstring &unpadded, sstring &padded);

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
  
  const uint8_t *GetNonce();

private:
  GenRandomFn genRandom;
  void * const genRandomData;
  uint8_t nonce[64];
  uint64_t ts_request;


};

#endif // #ifdef __cplusplus
#endif
