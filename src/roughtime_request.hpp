#ifndef ROUGHTIME_REQUEST_HPP
#define ROUGHTIME_REQUEST_HPP

#ifdef __cplusplus
#include "roughtime_common.hpp"

class RtClient {
public:

  // Do not use this struct directly; it is only here for documentation
  // purposes.
  typedef struct RtHeaderTag {
    uint32_t num_tags_le;
    uint32_t offets[1]; // offsets[MAX(0, num_tags-1)]
    uint32_t tags[1];   // tags[num_tags]
  } RtHeaderT;

  RtClient();
  ~RtClient();

  void GenerateRequest(
    sstring &request,
    const uint8_t *pNonce = nullptr,
    const size_t  nonceLen = 0
  );

  static void PadRequest(const sstring &unpadded, sstring &padded);

  const uint8_t *GetNonce();

private:
  uint8_t nonce[64];
  uint64_t ts_request;
};

#endif // #ifdef __cplusplus

#endif