#pragma once

#include "cs_task_locker.hpp"
#include <cstdint>
#include <queue>

#ifndef MIN
#define MIN(x,y) (((x) < (y)) ? (x) : (y))
#endif

typedef std::queue<uint8_t> Queue;

class ByteQ {
public:
  ByteQ()
  {
    
  }
  
  size_t Write(const uint8_t bytes[], const size_t len){
    CSTaskLocker cs;
    for (size_t i = 0; i < len; i++){
      mQueue.push(bytes[i]);
    }
    return len;
  }
  
  size_t GetWriteReady(){
    return 4096;
  }
  
  size_t GetReadReady(){
    return mQueue.size();
  }
  
  size_t Read(uint8_t bytes[], const size_t len){
    CSTaskLocker cs;
    size_t readBytes = GetReadReady();
    readBytes = MIN(len, readBytes);
    for (size_t i = 0; i < readBytes; i++){
      bytes[i] = mQueue.front();
      mQueue.pop();
    }
    return readBytes;
  }
  
private:
  Queue mQueue;
  
};
