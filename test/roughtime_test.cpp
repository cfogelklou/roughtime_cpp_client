#include <curl/curl.h>
#include <curl/easy.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
using namespace testing;

TEST(TestCurl, curl_1) {
  CURL * curl = curl_easy_init();
  EXPECT_NE(nullptr, curl);
  curl_easy_cleanup(curl);
  
}

int main(int argc, char** argv){
  
  // The following line must be executed to initialize Google Mock
  // (and Google Test) before running the tests.
  ::testing::InitGoogleMock(&argc, argv);
  const int gtest_rval = RUN_ALL_TESTS();
  
  return gtest_rval;
}
