
#include "roughtime.hpp"
#include <string>
#include <chrono>
#include <assert.h>



// ////////////////////////////////////////////////////////////////////////////
static uint64_t rt_seconds_since_epoch() {
  auto now = std::chrono::duration_cast<std::chrono::milliseconds>
    (std::chrono::system_clock::now().time_since_epoch()).count();
  return (uint64_t)now;
}

// /////////////////////////////////////////////////////////////////////////////
RtClient::RtClient()
: nonce()
, ts_request(0)
{
}

// /////////////////////////////////////////////////////////////////////////////
RtClient::~RtClient(){
  
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
void RtClient::GenerateRequest(std::ustring &request){
  uint64_t ts = rt_seconds_since_epoch();
  
  RtRequestT req;
  req.req.num_tags_le = HOSTTOLE32(2);
  req.req.offset1_le = HOSTTOLE32(64); // Padding starts at offset 64
  req.req.nonce_le = HOSTTOLE32(roughtime::NONC);
  req.req.pad_le = HOSTTOLE32(roughtime::PAD);
  memset(nonce, 0, sizeof(nonce));
  memcpy(nonce, &ts, sizeof(ts));
  memcpy(req.req.nonce, nonce, sizeof(nonce));
  request.assign(req.u, sizeof(req));
}

// /////////////////////////////////////////////////////////////////////////////
void RtClient::PadRequest(
  const std::ustring &unpadded,
  std::ustring &padded)
{
  if (((void *)&unpadded) != ((void *)&padded)){
    padded.clear();
    padded.insert(0, unpadded.data(), sizeof(RtRequestT));
  }
  else {
    std::ustring tmp = unpadded;
    padded.clear();
    padded.insert(0, tmp.data(), sizeof(RtRequestT));
  }
  uint8_t effeff[1024-sizeof(RtRequestT)];
  memset(effeff, 0xff, sizeof(effeff));
  padded.insert(sizeof(RtRequestT), effeff, sizeof(effeff));

}
