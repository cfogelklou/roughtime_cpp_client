#include "roughtime_request.hpp"
#include "roughtime_common.hpp"
#include "roughtime_private.hpp"
#include "crypto_sign.h"

static void stupidRandom(uint8_t *buf, int cnt) {
  for (int i = 0; i < cnt; i++) {
    buf[i] = rand() % 255;
  }
}

typedef union RtRequestTag {
  struct {
    uint32_t num_tags_le;
    uint32_t offset1_le;
    uint32_t nonce_le;
    uint32_t pad_le;
    uint8_t  nonce[64];
  } req;
  uint8_t u[80];
} RpRequestT;

// /////////////////////////////////////////////////////////////////////////////
void RoughTime::GenerateRequest(
  sstring &request,
  const uint8_t *pNonce,
  const size_t  nonceLen
) {
  uint8_t nonce[64] = { 0 };

  if (pNonce && (nonceLen > 0)) {
    const size_t len = MIN(nonceLen, sizeof(nonce));
    memcpy(nonce, pNonce, len);
  }
  else {
    stupidRandom(nonce, 64);
  }

  RpRequestT req;
  req.req.num_tags_le = HOSTTOLE32(2);
  req.req.offset1_le = HOSTTOLE32(64); // Padding starts at offset 64
  req.req.nonce_le = HOSTTOLE32(RoughTime::NONC);
  req.req.pad_le = HOSTTOLE32(RoughTime::PAD);
  memcpy(req.req.nonce, nonce, sizeof(nonce));
  request.assign(req.u, sizeof(req));
}

// /////////////////////////////////////////////////////////////////////////////
void RoughTime::PadRequest(
  const sstring &unpadded,
  sstring &padded)
{
  if (((void *)&unpadded) != ((void *)&padded)) {
    padded.clear();
    padded.assign(unpadded.u_str(), sizeof(RpRequestT));
  }
  else {
    sstring tmp = unpadded;
    padded.clear();
    padded.assign(tmp.u_str(), sizeof(RpRequestT));
  }
  uint8_t effeff[1024 - sizeof(RpRequestT)];
  memset(effeff, 0xff, sizeof(effeff));
  padded.append(effeff, sizeof(effeff));

}
