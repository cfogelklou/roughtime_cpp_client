#pragma once

#include <cstdint>
#include <cstdlib>

class QueueBase {
public:
  QueueBase(){}
  virtual ~QueueBase(){}
  virtual size_t Write(const uint8_t arr[], const size_t len){
    return 0;
  }
  virtual size_t Read(uint8_t arr[], const size_t len){
    return 0;
  }
  virtual size_t GetWriteReady(){
    return 0;
  }
  virtual size_t GetReadReady(){
    return 0;
  }
};
