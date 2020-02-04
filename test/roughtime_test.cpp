#define USE_CURL 0

#include "src/roughtime_request.hpp"
#include "src/roughtime_parse.hpp"
#include "utils/platform_log.h"
#include "buf_io/buf_io_queue.hpp"
#include "src/roughtime_servers.hpp"

LOG_MODNAME("roughtime_test.cpp");

#if (USE_CURL > 0)
#include <curl/curl.h>
#include <curl/easy.h>
#endif
#include <gtest/gtest.h>
#include <gmock/gmock.h>
using namespace testing;

#ifdef WIN32
#include <Windows.h>
#define usleep(x) Sleep(x/1000)
#endif

#if (USE_CURL > 0)
TEST(TestCurl, curl_1) {
  CURL * curl = curl_easy_init();
  EXPECT_NE(nullptr, curl);
  curl_easy_cleanup(curl);  
}
#endif


static void stupidRandom(uint8_t *buf, int cnt) {
  for (int i = 0; i < cnt; i++) {
    buf[i] = rand() % 255;
  }
}

TEST(TestRt, UnpaddedRequest){
  sstring req;
  uint8_t nonce[64];
  stupidRandom(nonce, sizeof(nonce));
  RoughTime::GenerateRequest(req, nonce, sizeof(nonce));
  ASSERT_EQ(req.length(), 80);
  const uint8_t* pr = req.u_str();
  EXPECT_EQ(pr[0], 2); // 2 tags
  EXPECT_EQ(pr[1], 0);
  EXPECT_EQ(pr[2], 0);
  EXPECT_EQ(pr[3], 0);
  EXPECT_EQ(pr[4], 64); // Second tag has offset 64
  EXPECT_EQ(pr[5], 0);
  EXPECT_EQ(pr[6], 0);
  EXPECT_EQ(pr[7], 0);
  EXPECT_EQ(pr[8], 'N'); // First data (at offset zero) has tag nonc
  EXPECT_EQ(pr[9], 'O');
  EXPECT_EQ(pr[10], 'N');
  EXPECT_EQ(pr[11], 'C');
  EXPECT_EQ(pr[12], 'P'); // Second data (at offset 64) has tag PAD
  EXPECT_EQ(pr[13], 'A');
  EXPECT_EQ(pr[14], 'D');
  EXPECT_EQ(pr[15], 0xff);

}

TEST(TestRt, PaddedRequest){
  sstring req;
  uint8_t nonce[64];
  stupidRandom(nonce, sizeof(nonce));
  RoughTime::GenerateRequest(req, nonce, sizeof(nonce));
  ASSERT_EQ(req.length(), 80);
  RoughTime::PadRequest(req, req);
  ASSERT_EQ(req.length(), 1024);
  const uint8_t *pr = req.u_str();
  EXPECT_EQ(pr[0], 2); // 2 tags
  EXPECT_EQ(pr[1], 0);
  EXPECT_EQ(pr[2], 0);
  EXPECT_EQ(pr[3], 0);
  EXPECT_EQ(pr[4], 64); // Second tag has offset 64
  EXPECT_EQ(pr[5], 0);
  EXPECT_EQ(pr[6], 0);
  EXPECT_EQ(pr[7], 0);
  EXPECT_EQ(pr[8], 'N'); // First data (at offset zero) has tag nonc
  EXPECT_EQ(pr[9], 'O');
  EXPECT_EQ(pr[10], 'N');
  EXPECT_EQ(pr[11], 'C');
  EXPECT_EQ(pr[12], 'P'); // Second data (at offset 64) has tag PAD
  EXPECT_EQ(pr[13], 'A');
  EXPECT_EQ(pr[14], 'D');
  EXPECT_EQ(pr[15], 0xff);

  EXPECT_EQ(pr[1023], 0xff);
}

TEST(TestRt, RoughtimeParse) {
  const uint8_t nonce[] = { 5,83,77,186,201,145,69,152,83,175,116,202,170,224,249,101,238,22,56,197,147,36,14,207,95,96,87,228,209,115,210,207,11,73,211,160,163,96,120,215,35,70,189,113,239,168,19,66,87,253,196,183,159,189,59,195,190,205,149,160,113,25,228,80 };

  EXPECT_EQ(sizeof(nonce) , 64);

  const uint8_t packet[] = { 2,0,0,0,64,0,0,0,78,79,78,67,80,65,68,255,5,83,77,186,201,145,69,152,83,175,116,202,170,224,249,101,238,22,56,197,147,36,14,207,95,96,87,228,209,115,210,207,11,73,211,160,163,96,120,215,35,70,189,113,239,168,19,66,87,253,196,183,159,189,59,195,190,205,149,160,113,25,228,80,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
  EXPECT_EQ(sizeof(packet) , 1024);

  const uint8_t answer[] = { 5,0,0,0,64,0,0,0,64,0,0,0,164,0,0,0,60,1,0,0,83,73,71,0,80,65,84,72,83,82,69,80,67,69,82,84,73,78,68,88,73,155,232,86,166,133,223,183,220,49,82,201,230,189,142,241,222,152,21,121,97,17,75,234,111,156,147,90,106,7,48,125,177,46,67,105,6,109,248,234,16,74,219,210,232,133,135,104,159,76,117,3,78,81,60,143,146,168,252,219,110,156,119,8,3,0,0,0,4,0,0,0,12,0,0,0,82,65,68,73,77,73,68,80,82,79,79,84,64,66,15,0,216,217,100,144,127,131,5,0,141,188,245,106,82,96,124,212,119,88,228,116,35,184,129,193,14,215,109,9,121,26,34,251,219,96,97,97,30,98,245,79,94,255,156,34,30,190,95,254,163,232,173,188,236,78,119,119,126,255,231,14,106,5,217,89,187,226,103,228,225,232,87,90,2,0,0,0,64,0,0,0,83,73,71,0,68,69,76,69,136,199,40,177,0,54,169,60,144,114,166,1,236,0,51,68,40,41,2,246,32,74,27,154,240,238,32,180,205,203,87,222,183,178,218,164,235,190,114,157,246,106,48,40,136,132,197,142,165,56,245,236,230,253,140,206,151,136,82,39,134,200,155,15,3,0,0,0,32,0,0,0,40,0,0,0,80,85,66,75,77,73,78,84,77,65,88,84,3,197,172,255,171,147,174,153,132,224,166,216,116,206,1,95,50,69,25,215,47,249,61,190,241,62,229,10,115,56,255,216,48,213,14,9,115,131,5,0,48,53,230,38,135,131,5,0,0,0,0,0 };
  EXPECT_EQ(sizeof(answer), 360);

  const uint8_t pubkey[] = { 128, 62, 183, 133, 40, 247, 73, 196, 190, 194, 227, 158, 26, 187, 155, 94, 90, 183, 228, 221, 92, 228, 182, 242, 253, 47, 147, 236, 195, 83, 143, 26 };
  EXPECT_EQ(sizeof(pubkey), 32);

  const uint64_t midpoint = 1551958790167000;
  const uint64_t radius = 1000000;

  RoughTime::ParseOutT times;
  EXPECT_EQ(midpoint, RoughTime::ParseToMicroseconds(pubkey, nonce, answer, sizeof(answer), &times));

  EXPECT_EQ(times.radius, radius);
  EXPECT_EQ(times.midpoint, midpoint);

}

#include "mini_socket/mini_socket.hpp"

TEST(TestRt, SendRequestOk){
  sstring req;
  uint8_t nonce[64];
  stupidRandom(nonce, sizeof(nonce));
  RoughTime::GenerateRequest(req, nonce, sizeof(nonce));
  ASSERT_EQ(req.length(), 80);
  RoughTime::PadRequest(req, req);
  ASSERT_EQ(req.length(), 1024);
  const uint8_t* pr = req.u_str();
  EXPECT_EQ(pr[0], 2); // 2 tags

  sstring rxBuf;

  auto onSocketRead = [](struct BufIOQTransTag* pTransaction) {
    uint8_t* nonce = (uint8_t * )pTransaction->pUserData;
    auto bytes = pTransaction->transferredIdx;
    const uint8_t* const buf = pTransaction->pBuf8;
    EXPECT_GT(bytes, 0u);
    if (bytes) {

      uint8_t pubkey[32];
      const RoughtimeServer * srv = RoughtimeGetServerAtIdx(0);
      RoughtimeGetKey(srv, pubkey);
      RoughTime::ParseOutT times;

      uint64_t midpoint = RoughTime::ParseToMicroseconds(pubkey, nonce, buf, bytes, &times);
      midpoint /= (1000ull * 1000ull);

      std::time_t now = std::time(0);
      uint64_t compareTo = (uint64_t)now;
      auto diff = fabs(compareTo - midpoint);
      EXPECT_LE(diff, 2000);

      // Indicate that time was ok.
      nonce[0] = 0;

    }
  };

  const RoughtimeServer * srv = RoughtimeGetServerAtIdx(0);
  
  BufIOQTransT rxTrans(rxBuf.u8DataPtr(1024, 1024), 1024, onSocketRead, nonce);
  
  BufIOQueue *ptr = CreateUdpClient(srv->addr, srv->port);
  EXPECT_TRUE(ptr != nullptr);
  LOG_ASSERT(ptr);
  BufIOQueue& p = *ptr;
  p.QueueRead(&rxTrans);
  p.Write(req.u_str(), req.length());
  int tries = 0;
  while ((tries < 1000) && (nonce[0])){
    usleep(10000);
  }
  DeleteUdpClient(&p);
     
}

TEST(TestRt, SendRequestError){
  sstring req;
  uint8_t nonce[64];
  stupidRandom(nonce, sizeof(nonce));
  RoughTime::GenerateRequest(req, nonce, sizeof(nonce));
  ASSERT_EQ(req.length(), 80);
  RoughTime::PadRequest(req, req);
  ASSERT_EQ(req.length(), 1024);
  const uint8_t* pr = req.u_str();
  EXPECT_EQ(pr[0], 2); // 2 tags

  sstring rxBuf;

  auto onSocketRead = [](struct BufIOQTransTag* pTransaction) {
    uint8_t* nonce = (uint8_t * )pTransaction->pUserData;
    auto bytes = pTransaction->transferredIdx;
    const uint8_t* const buf = pTransaction->pBuf8;
    EXPECT_GT(bytes, 0u);
    if (bytes) {

      uint8_t pubkey[32];
      const RoughtimeServer * srv = RoughtimeGetServerAtIdx(0);
      RoughtimeGetKey(srv, pubkey);
      
      LOG_TRACE(("Corrupting 1 bit of the public key to confirm that times don't verify\r\n"));
      pubkey[12] = pubkey[12] ^ 0x01;
      RoughTime::ParseOutT times;

      uint64_t midpoint = RoughTime::ParseToMicroseconds(pubkey, nonce, buf, bytes, &times);
      midpoint /= (1000ull * 1000ull);

      std::time_t now = std::time(0);
      uint64_t compareTo = (uint64_t)now;
      auto diff = fabs(compareTo - midpoint);
      EXPECT_GT(diff, 10000000);

      // Indicate that time was ok.
      nonce[0] = 0;

    }
  };

  const RoughtimeServer * srv = RoughtimeGetServerAtIdx(0);
  
  BufIOQTransT rxTrans(rxBuf.u8DataPtr(1024, 1024), 1024, onSocketRead, nonce);
  
  BufIOQueue *ptr = CreateUdpClient(srv->addr, srv->port);
  EXPECT_TRUE(ptr != nullptr);
  LOG_ASSERT(ptr);
  BufIOQueue& p = *ptr;
  p.QueueRead(&rxTrans);
  p.Write(req.u_str(), req.length());
  int tries = 0;
  while ((tries < 1000) && (nonce[0])){
    usleep(10000);
  }
  DeleteUdpClient(&p);
     
}

int main(int argc, char** argv){
  
  // The following line must be executed to initialize Google Mock
  // (and Google Test) before running the tests.
  ::testing::InitGoogleMock(&argc, argv);
  const int gtest_rval = RUN_ALL_TESTS();
  
  return gtest_rval;
}
