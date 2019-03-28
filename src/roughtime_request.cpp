#include "roughtime_request.hpp"
#include "roughtime_common.hpp"
#include "roughtime_private.hpp"
#include "crypto_sign.h"

void stupidRandom(uint8_t *buf, int cnt) {
  for (int i = 0; i < cnt; i++) {
    buf[i] = rand() % 255;
  }
}

// /////////////////////////////////////////////////////////////////////////////
RtClient::RtClient(
)
  : nonce()
  , ts_request(0)
{
}

// /////////////////////////////////////////////////////////////////////////////
RtClient::~RtClient() {

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
} RtRequestT;

// /////////////////////////////////////////////////////////////////////////////
void RtClient::GenerateRequest(
  sstring &request,
  const uint8_t *pNonce,
  const size_t  nonceLen
) {

  memset(nonce, 0, sizeof(nonce));
  if (pNonce && (nonceLen > 0)) {
    const size_t len = MIN(nonceLen, sizeof(this->nonce));
    memcpy(nonce, pNonce, len);
  }
  else {
    stupidRandom(nonce, 64);
  }

  RtRequestT req;
  req.req.num_tags_le = HOSTTOLE32(2);
  req.req.offset1_le = HOSTTOLE32(64); // Padding starts at offset 64
  req.req.nonce_le = HOSTTOLE32(roughtime::NONC);
  req.req.pad_le = HOSTTOLE32(roughtime::PAD);
  memcpy(req.req.nonce, nonce, sizeof(nonce));
  request.assign(req.u, sizeof(req));
}

// /////////////////////////////////////////////////////////////////////////////
void RtClient::PadRequest(
  const sstring &unpadded,
  sstring &padded)
{
  if (((void *)&unpadded) != ((void *)&padded)) {
    padded.clear();
    padded.assign(unpadded.u_str(), sizeof(RtRequestT));
  }
  else {
    sstring tmp = unpadded;
    padded.clear();
    padded.assign(tmp.u_str(), sizeof(RtRequestT));
  }
  uint8_t effeff[1024 - sizeof(RtRequestT)];
  memset(effeff, 0xff, sizeof(effeff));
  padded.append(effeff, sizeof(effeff));

}

// /////////////////////////////////////////////////////////////////////////////
static int reject(const uint8_t b[], const char *message) {
  (void)b;
  LOG_WARNING(("roughtime::rejected due to %s\r\n", message));
  return -1;
}

// /////////////////////////////////////////////////////////////////////////////
static uint32_t uint32(const uint8_t b[], const int i) {
  uint32_t tmp;
  memcpy(&tmp, &b[i], sizeof(tmp));
  return LE32TOHOST(tmp);
}

// /////////////////////////////////////////////////////////////////////////////
static uint64_t uint64(const uint8_t b[], const int i) {
  uint64_t tmp;
  memcpy(&tmp, &b[i], sizeof(tmp));
  return LE64TOHOST(tmp);
}

// /////////////////////////////////////////////////////////////////////////////
static const uint8_t * subarray(
  const sstring &bstr,
  const size_t fromIdx,
  const size_t toIdx, // up to AND INCLUDING
  sstring &out) {
  const size_t end = MIN(toIdx, (bstr.length() - 1));
  const size_t beg = MIN(end, fromIdx);
  const int len = end - beg;
  LOG_ASSERT(len >= 0);
  out.clear();
  const uint8_t * const b = bstr.u_str();
  out.assign(&b[beg], len);

  LOG_ASSERT_WARN(out.length() == (toIdx - fromIdx));
  return out.u_str();
}


// /////////////////////////////////////////////////////////////////////////////
static bool verify
(
  const sstring &sigstr,
  const sstring &prefix,
  const sstring &bstring,
  const int start,
  const int end,
  const sstring &pubkey
)
{
  sstring signedStr;
  subarray(bstring, start, end, signedStr);

  sstring scratch1(prefix);
  scratch1.append(signedStr);

  int noob = crypto_sign_verify_detached(
    sigstr.u_str(),
    scratch1.u_str(),
    scratch1.length(),
    pubkey.u_str());

  return (0 == noob);

}

// /////////////////////////////////////////////////////////////////////////////
const uint8_t *RtClient::GetNonce() {
  return nonce;
}
