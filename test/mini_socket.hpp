#pragma once

#include "queue_base.hpp"

QueueBase &CreateUdpClient(const char *szAddr, const int port);

void DeleteUdpClient(QueueBase *p);

