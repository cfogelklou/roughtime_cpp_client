#pragma once

#include "cs_task_locker.hpp"
#include "queue_base.hpp"
#include <cstdint>
#include <queue>

#ifndef MIN
#define MIN(x,y) (((x) < (y)) ? (x) : (y))
#endif

typedef std::queue<uint8_t> Queue;

class ByteQ : public QueueBase {
public:
  ByteQ()
  : mQueue()
  {
  }
  
  size_t Write(const uint8_t bytes[], const size_t len) override {
    CSTaskLocker cs;
    for (size_t i = 0; i < len; i++){
      mQueue.push(bytes[i]);
    }
    return len;
  }
  
  size_t GetWriteReady() override {
    return 4096;
  }
  
  size_t GetReadReady() override {
    return mQueue.size();
  }
  
  size_t Read(uint8_t bytes[], const size_t len) override {
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
