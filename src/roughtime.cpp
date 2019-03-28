#include "roughtime.hpp"
#include "crypto_sign.h"
#ifdef __POLESTAR_PAK
#include "utils/ble_log.h"
#include "utils/helper_macros.h"
#include "platform/endian_convert.h"

LOG_MODNAME("roughtime.cpp");
#else
#include <assert.h>
#include "endian_convert.h"
#define u_str() data()
#define MIN(x,y) (((x) < (y)) ? (x) : (y))
#define LOG_WARNING(x) printf x
#define LOG_ASSERT assert
#define LOG_ASSERT_WARN(x) if (!(x)) do { printf("Warning at %s (%d)\r\n", __FILE__, __LINE__);} while(0)
#endif

// /////////////////////////////////////////////////////////////////////////////
RtClient::RtClient(
  GenRandomFn genRandom,
  void *genRandomData
)
: genRandom(genRandom)
, genRandomData(genRandomData)
, nonce()
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
void RtClient::GenerateRequest(
  sstring &request,
  const uint8_t *pNonce ,
  const size_t  nonceLen
  ){
  
  memset(nonce, 0, sizeof(nonce));
  if (pNonce && (nonceLen > 0)) {
    const size_t len = MIN(nonceLen, sizeof(this->nonce));
    memcpy(nonce, pNonce, len);
  }
  else if (genRandom) {
    genRandom(genRandomData, nonce, sizeof(nonce));
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
  if (((void *)&unpadded) != ((void *)&padded)){
    padded.clear();
    padded.assign(unpadded.u_str(), sizeof(RtRequestT));
  }
  else {
    sstring tmp = unpadded;
    padded.clear();
    padded.assign(tmp.u_str(), sizeof(RtRequestT));
  }
  uint8_t effeff[1024-sizeof(RtRequestT)];
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

// /////////////////////////////////////////////////////////////////////////////
static const char certificateContext[] = "RoughTime v1 delegation signature--";
static const char signedResponseContext[] = "RoughTime v1 response signature";

// /////////////////////////////////////////////////////////////////////////////
uint64_t RtClient::Parse(
  const uint8_t pubkey[32],
  const uint8_t nonce[64],
  const uint8_t b[],
  const size_t b_length,
  ParseOutT * const pOut
) {
  sstring bstring;
  bstring.assign(b, b_length);

  int CERT_tagstart = -1;
  int CERT_tagend = -1;
  int INDX_tagstart = -1;
  int INDX_tagend = -1;
  int PATH_tagstart = -1;
  int PATH_tagend = -1;
  int SIG_tagstart = -1;
  int SIG_tagend = -1;
  int SREP_tagstart = -1;
  int SREP_tagend = -1;
  int CERT_DELE_tagstart = -1;
  int CERT_DELE_tagend = -1;
  int CERT_SIG_tagstart = -1;
  int CERT_SIG_tagend = -1;
  int CERT_DELE_MAXT_tagstart = -1;
  int CERT_DELE_MAXT_tagend = -1;
  int CERT_DELE_MINT_tagstart = -1;
  int CERT_DELE_MINT_tagend = -1;
  int CERT_DELE_PUBK_tagstart = -1;
  int CERT_DELE_PUBK_tagend = -1;
  int SREP_MIDP_tagstart = -1;
  int SREP_MIDP_tagend = -1;
  int SREP_RADI_tagstart = -1;
  int SREP_RADI_tagend = -1;
  int SREP_ROOT_tagstart = -1;
  int SREP_ROOT_tagend = -1;

  int s = 0;
  int i = 0;
  int n = b_length;

  if (n % 4 > 0) {
    return reject(b, "short message");
  }

  bool done = false;
  while (!done) {
    if (i + 4 > n) {
      return reject(b, "short message");
    }

    const uint32_t ntags = uint32(b, i);
    i += 4;

    if (ntags == 0) {
      return reject(b, "no tags"); // not technically illegal but...
    }

    const int firstoffset = i;
    i += 4 * (ntags - 1);
    const int lastoffset = i;

    const int firsttag = i;
    i += 4 * ntags;
    const int lasttag = i;

    const int firstdatabyte = i;

    if (i > n) {
      return reject(b, "short message");
    }

    for (int last = -1, j = firstoffset; j < lastoffset; j += 4) {
      const int32_t offset = uint32(b, j);

      if (offset < last) {
        return reject(b, "illegal offset (order)");
      }
      last = offset;

      if (offset % 4 > 0) {
        return reject(b, "illegal offset (alignment)");
      }

      if (offset + firstdatabyte > n) {
        return reject(b, "illegal offset (range)");
      }
    }

    int tagstart = firstdatabyte;
    int tagend = -1;

    for (int j = firsttag; j < lasttag; j += 4, tagstart = tagend) {
      tagend = n;

      if (j + 4 < lasttag) {
        tagend = firstdatabyte + uint32(b, j - firsttag + firstoffset);
      }

      const uint32_t tag = uint32(b, j);

      switch (s) {
      case 0: // toplevel
        switch (tag) {
        case roughtime::CERT:
          CERT_tagstart = tagstart;
          CERT_tagend = tagend;
          break;
        case roughtime::INDX:
          INDX_tagstart = tagstart;
          INDX_tagend = tagend;
          break;
        case roughtime::PATH:
          PATH_tagstart = tagstart;
          PATH_tagend = tagend;
          break;
        case roughtime::SIG:
          SIG_tagstart = tagstart;
          SIG_tagend = tagend;
          break;
        case roughtime::SREP:
          SREP_tagstart = tagstart;
          SREP_tagend = tagend;
          break;
        }
        break; // case 0:

      case 1: // CERT
        switch (tag) {
        case roughtime::DELE:
          CERT_DELE_tagstart = tagstart;
          CERT_DELE_tagend = tagend;
          break;
        case roughtime::SIG:
          CERT_SIG_tagstart = tagstart;
          CERT_SIG_tagend = tagend;
          break;
        }
        break; // case 1: // CERT

      case 2: // CERT_DELE
        switch (tag) {
        case roughtime::MAXT:
          CERT_DELE_MAXT_tagstart = tagstart;
          CERT_DELE_MAXT_tagend = tagend;
          break;
        case roughtime::MINT:
          CERT_DELE_MINT_tagstart = tagstart;
          CERT_DELE_MINT_tagend = tagend;
          break;
        case roughtime::PUBK:
          CERT_DELE_PUBK_tagstart = tagstart;
          CERT_DELE_PUBK_tagend = tagend;
          break;
        }
        break; // case 2: // CERT_DELE

      case 3: // SREP
        switch (tag) {
        case roughtime::MIDP:
          SREP_MIDP_tagstart = tagstart;
          SREP_MIDP_tagend = tagend;
          break;
        case roughtime::RADI:
          SREP_RADI_tagstart = tagstart;
          SREP_RADI_tagend = tagend;
          break;
        case roughtime::ROOT:
          SREP_ROOT_tagstart = tagstart;
          SREP_ROOT_tagend = tagend;
          break;
        }
        break;
      }
    } // for (int j = firsttag; j < lasttag; j += 4, tagstart = tagend) {

    switch (s) {
    case 0: // toplevel
      if (CERT_tagstart < 0) {
        return reject(b, "no CERT tag");
      }

      if (INDX_tagstart < 0) {
        return reject(b, "no INDX tag");
      }

      if (INDX_tagend - INDX_tagstart != 4) {
        return reject(b, "bad INDX tag");
      }

      if (PATH_tagstart < 0) {
        return reject(b, "no PATH tag");
      }

      if ((PATH_tagend - PATH_tagstart) % 64 != 0) {
        return reject(b, "bad PATH tag");
      }

      if (SIG_tagstart < 0) {
        return reject(b, "no SIG tag");
      }

      if (SIG_tagend - SIG_tagstart != 64) {
        return reject(b, "bad SIG tag");
      }

      if (SREP_tagstart < 0) {
        return reject(b, "no SREP tag");
      }

      i = CERT_tagstart;
      n = CERT_tagend;
      s++; // CERT
      break;

    case 1: // CERT
      if (CERT_DELE_tagstart < 0) {
        return reject(b, "no CERT.DELE tag");
      }

      if (CERT_SIG_tagstart < 0) {
        return reject(b, "no CERT.SIG tag");
      }

      if (CERT_SIG_tagend - CERT_SIG_tagstart != 64) {
        return reject(b, "bad CERT.SIG tag");
      }

      i = CERT_DELE_tagstart;
      n = CERT_DELE_tagend;
      s++; // CERT_DELE
      break;

    case 2: // CERT_DELE
      if (CERT_DELE_MAXT_tagstart < 0) {
        return reject(b, "no CERT.DELE.MAXT tag");
      }

      if (CERT_DELE_MAXT_tagend - CERT_DELE_MAXT_tagstart != 8) {
        return reject(b, "bad CERT.DELE.MAXT tag");
      }

      if (CERT_DELE_MINT_tagstart < 0) {
        return reject(b, "no CERT.DELE.MINT tag");
      }

      if (CERT_DELE_MINT_tagend - CERT_DELE_MINT_tagstart != 8) {
        return reject(b, "bad CERT.DELE.MAXT tag");
      }

      if (CERT_DELE_PUBK_tagstart < 0) {
        return reject(b, "no CERT.DELE.PUBK tag");
      }

      if (CERT_DELE_PUBK_tagend - CERT_DELE_PUBK_tagstart != 32) {
        return reject(b, "bad CERT.DELE.PUBK");
      }

      i = SREP_tagstart;
      n = SREP_tagend;
      s++; // SREP
      break;

    case 3: // SREP
      if (SREP_MIDP_tagstart < 0) {
        return reject(b, "no SREP.MIDP tag");
      }

      if (SREP_MIDP_tagend - SREP_MIDP_tagstart != 8) {
        return reject(b, "bad SREP.MIDP tag");
      }

      if (SREP_RADI_tagstart < 0) {
        return reject(b, "no SREP.RADI tag");
      }

      if (SREP_RADI_tagend - SREP_RADI_tagstart != 4) {
        return reject(b, "bad SREP.RADI tag");
      }

      if (SREP_ROOT_tagstart < 0) {
        return reject(b, "no SREP.ROOT tag");
      }

      if (SREP_ROOT_tagend - SREP_ROOT_tagstart != 64) {
        return reject(b, "bad SREP.ROOT tag");
      }

      done = true;
      break /*done*/;
    }
  } // for (;;)

  {
    sstring sigstr;
    subarray(bstring, CERT_SIG_tagstart, CERT_SIG_tagend, sigstr);

    sstring pubkeystr;
    pubkeystr.assign(pubkey, 32);
    sstring delegate;
    sstring certificateContextStr((uint8_t *)certificateContext, strlen(certificateContext) + 1);
    if (!verify(sigstr, certificateContextStr, bstring, CERT_DELE_tagstart, CERT_DELE_tagend, pubkeystr)) {
      return reject(b, "CERT.DELE does not verify");
    }
  }

  {
    sstring sigstr;
    subarray(bstring, SIG_tagstart, SIG_tagend, sigstr);

    sstring pubkeystr;
    subarray(bstring, CERT_DELE_PUBK_tagstart, CERT_DELE_PUBK_tagend, pubkeystr);

    sstring signedResponseContextStr((uint8_t *)signedResponseContext, strlen(signedResponseContext) + 1);
    if (!verify(sigstr, signedResponseContextStr, bstring, SREP_tagstart, SREP_tagend, pubkeystr)) {
      return reject(b, "SREP does not verify");
    }
  }

  //let h = createHash("sha512").update(zero).update(nonce).digest()
  uint8_t h[512 / 8] = { 0 };
  {
    crypto_hash_sha512_state sha;
    crypto_hash_sha512_init(&sha);
    const uint8_t zero[1] = { 0 };
    crypto_hash_sha512_update(&sha, zero, sizeof(zero));
    crypto_hash_sha512_update(&sha, nonce, 64);
    crypto_hash_sha512_final(&sha, h);
  }
#if 0
  for (int i = 0; i < sizeof(h); i += 8) {
    printf("%d, %d, %d, %d, %d, %d, %d, %d, \r\n"
      , h[i + 0], h[i + 1], h[i + 2], h[i + 3]
      , h[i + 4], h[i + 5], h[i + 6], h[i + 7]
    );
  }
  printf("\r\nhi\r\n");
#endif 

  // //////////////////////////////////////////////////////////////////////////
  // HERE BE DRAGONS!
  // This code is basically untested. It was lightly tested in the original
  // javascript, and evel less tested here.
  // //////////////////////////////////////////////////////////////////////////
  const auto pathlen = PATH_tagend - PATH_tagstart;
  if (pathlen > 0) {
    uint32_t index = uint32(b, INDX_tagstart);

    for (int j = 0; j < pathlen; j += 64) {
      sstring lStr;
      subarray(bstring, PATH_tagstart + j, PATH_tagstart + j + 64, lStr);
      const uint8_t *l = lStr.u_str();

      //let r = h
      const uint8_t *r = h;

      if ((index & 1) == 0) {
        //[l, r] = [r, l]
        const uint8_t *lTmp = l;
        l = r;
        r = lTmp;
      }

      // h = createHash("sha512").update(one).update(l).update(r).digest()
      {
        crypto_hash_sha512_state sha;
        crypto_hash_sha512_init(&sha);
        const uint8_t one[1] = { 1 };
        crypto_hash_sha512_update(&sha, one, sizeof(one));
        crypto_hash_sha512_update(&sha, l, 64);
        crypto_hash_sha512_update(&sha, r, 64);
        crypto_hash_sha512_final(&sha, h);
      }

#if 0
      for (int i = 0; i < sizeof(h); i += 8) {
        printf("%d, %d, %d, %d, %d, %d, %d, %d, \r\n"
          , h[i + 0], h[i + 1], h[i + 2], h[i + 3]
          , h[i + 4], h[i + 5], h[i + 6], h[i + 7]
        );
      }
#endif 
      index >>= 1;
    }
  }
  // //////////////////////////////////////////////////////////////////////////
  // END OF HERE BE DRAGONS!
  // //////////////////////////////////////////////////////////////////////////

  {
    uint32_t i = 0;

    for (int j = 0; j < 64; j++) {
      i ^= h[j];
      i ^= b[j + SREP_ROOT_tagstart];
    }

    if (i != 0) {
      return reject(b, "ROOT does not verify");
    }
  }

  uint64_t midpoint, mintime, maxtime;
  const uint64_t deepFuture = 2097152ull << 32ull;
    // TODO(bnoordhuis) switch to BigInt before 2255 AD
  midpoint = uint64(b, SREP_MIDP_tagstart);
  if (midpoint > deepFuture)
    return reject(b, "deep future midpoint");

  mintime = uint64(b, CERT_DELE_MINT_tagstart);
  if (mintime > deepFuture)
    return reject(b, "deep future mintime");

  maxtime = uint64(b, CERT_DELE_MAXT_tagstart);
  if (maxtime > deepFuture)
    return reject(b, "deep future maxtime");

  if (maxtime < mintime) {
    return reject(b, "maxtime < mintime");
  }

  if (midpoint < mintime) {
    return reject(b, "midpoint < mintime");
  }

  if (midpoint > maxtime) {
    return reject(b, "midpoint > maxtime");
  }
  
  const uint32_t radius = uint32(b, SREP_RADI_tagstart);

  if (pOut) {
    pOut->maxtime = maxtime;
    pOut->mintime = mintime;
    pOut->midpoint = midpoint;
    pOut->radius = radius;
  }

  return midpoint;// midpoint, radius, null]
    
}
