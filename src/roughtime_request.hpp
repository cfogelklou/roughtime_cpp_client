#ifndef ROUGHTIME_REQUEST_HPP
#define ROUGHTIME_REQUEST_HPP

#ifdef __cplusplus
#include "roughtime_common.hpp"

namespace RoughTime {

  void GenerateRequest(
    sstring &request,
    const uint8_t *pNonce = nullptr,
    const size_t  nonceLen = 0
  );

  void PadRequest(const sstring &unpadded, sstring &padded);


};

#endif // #ifdef __cplusplus

#endif