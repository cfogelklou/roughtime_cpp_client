#pragma once

#include "queue_base.hpp"

QueueBase &GetQueueToIp(const char *szAddr, const int port);

void ReleaseQueueToIp(QueueBase *p);

