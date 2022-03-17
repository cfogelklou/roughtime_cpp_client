#include "cs_task_locker.hpp"
#include <mutex>

static std::mutex csMutex;

// Mini Critical Section Locker using a mutex.
CSTaskLocker::CSTaskLocker() {
  csMutex.lock();
}
  
CSTaskLocker::~CSTaskLocker() {
  csMutex.unlock();
}
