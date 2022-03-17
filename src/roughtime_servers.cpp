/**
* COPYRIGHT	(c)	Applicaudia 2019
* @file     roughtime_servers.cpp
* @brief    Indexable list of roughtime servers trusted by the firmware.
*/

#include "roughtime_servers.hpp"
#include "roughtime_private.hpp"


extern "C" {

const RoughtimeServer RoughtimeServersArr[] = {
     { "roughtime.cloudflare.com",         2002, "803eb78528f749c4bec2e39e1abb9b5e5ab7e4dd5ce4b6f2fd2f93ecc3538f1a" }
    ,{ "roughtime.sandbox.google.com",     2002, "7ad3da688c5c04c635a14786a70bcf30224cc25455371bf9d4a2bfb64b682534" }
    ,{ "roughtime.int08h.com",             2002, "016e6e0284d24c37c6e4d7d8d5b4e1d3c1949ceaa545bf875616c9dce0c9bec1" }
    //,{ "roughtime.kyhwana.org",            2002, "f1992a67a9d14b662efa86cca3db62cfc2e48810cf45ba5df181d8fc135b8261" }
    //,{ "roughtime.blackhatspottycat.net",  2002, "2397e2512392ad9532341b0dbc3581a3a04dabffebf00bd0af8d6deac19071bc" }
};

const size_t RoughtimeServersCount = ARRSZ(RoughtimeServersArr);

// ////////////////////////////////////////////////////////////////////////////
const RoughtimeServer * RoughtimeGetServerAtIdx(const int idx) {
  if (idx < (int)RoughtimeServersCount) {
    return &RoughtimeServersArr[idx];
  }
  else {
    return nullptr;
  }
}


// ////////////////////////////////////////////////////////////////////////////
bool RoughtimeGetKey(const RoughtimeServer *p, uint8_t keyBin[32]) {

  bool rval = false;
  if (p) {
    int i = 0;
    for (int o = 0; o < 32; o++) {
      char msn = toupper(p->keyHex[i + 0]);
      char lsn = toupper(p->keyHex[i + 1]);
      uint8_t m = (msn >= 'A') ? msn - 'A' + 10 : msn - '0';
      uint8_t l = (lsn >= 'A') ? lsn - 'A' + 10 : lsn - '0';
      keyBin[o] = (m << 4) + l;
      i += 2;
    }
    rval = true;
  }
  return rval;
}

// ////////////////////////////////////////////////////////////////////////////
bool RoughtimeGetKeyAtIdx(const int idx, uint8_t keyBin[32]) {
  const RoughtimeServer * const p = RoughtimeGetServerAtIdx(idx);
  return RoughtimeGetKey(p, keyBin);
}

} // Extern "C"
