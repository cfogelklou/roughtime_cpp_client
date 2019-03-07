#define USE_CURL 0

#include "src/roughtime.hpp"

#if (USE_CURL > 0)
#include <curl/curl.h>
#include <curl/easy.h>
#endif
#include <gtest/gtest.h>
#include <gmock/gmock.h>
using namespace testing;

#if (USE_CURL > 0)
TEST(TestCurl, curl_1) {
  CURL * curl = curl_easy_init();
  EXPECT_NE(nullptr, curl);
  curl_easy_cleanup(curl);  
}
#endif

TEST(TestRt, Constructor){
  RtClient rt;
}

TEST(TestRt, UnpaddedRequest){
  RtClient rt;
  std::ustring req;
  rt.GenerateRequest(req);
  ASSERT_EQ(req.length(), 64 + 4);
  const uint8_t *pr = req.data();
  EXPECT_EQ(pr[0], 'N');
  EXPECT_EQ(pr[1], 'O');
  EXPECT_EQ(pr[2], 'N');
  EXPECT_EQ(pr[3], 'C');

}

TEST(TestRt, PaddedRequest){
  RtClient rt;
  std::ustring req;
  rt.GenerateRequest(req);
  ASSERT_EQ(req.length(), 64 + 4);
  rt.PadRequest(req, req);
  ASSERT_EQ(req.length(), 1024);
  const uint8_t *pr = req.data();
  EXPECT_EQ(pr[0], 'N');
  EXPECT_EQ(pr[1], 'O');
  EXPECT_EQ(pr[2], 'N');
  EXPECT_EQ(pr[3], 'C');
  EXPECT_EQ(pr[1023], 0xff);
}

#include "mini_socket.hpp"

TEST(TestRt, SendRequest){
  RtClient rt;
  std::ustring req;
  rt.GenerateRequest(req);
  ASSERT_EQ(req.length(), 64 + 4);
  rt.PadRequest(req, req);
  ASSERT_EQ(req.length(), 1024);
  const uint8_t *pr = req.data();
  EXPECT_EQ(pr[0], 'N');
  EXPECT_EQ(pr[1], 'O');
  EXPECT_EQ(pr[2], 'N');
  EXPECT_EQ(pr[3], 'C');

  QueueBase &p = GetQueueToIp("roughtime.cloudflare.com", 2002);
  p.Write(req.data(), req.length());
  while (!p.GetReadReady()){
    usleep(10000);
  }
  
  ReleaseQueueToIp(&p);
  
  
  
  
}

int main(int argc, char** argv){
  
  // The following line must be executed to initialize Google Mock
  // (and Google Test) before running the tests.
  ::testing::InitGoogleMock(&argc, argv);
  const int gtest_rval = RUN_ALL_TESTS();
  
  return gtest_rval;
}
