#pragma once

#include <cstdint>

// Mini Critical Section Locker using a mutex.
class CSTaskLocker {
public:
  CSTaskLocker();  
  ~CSTaskLocker();
};
