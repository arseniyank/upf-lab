#pragma once

#include "fd.h"  // Fd

Fd openRawSocket(const char* ifName);
Fd openGtpuSocket();
bool setNonBlocking(int fd);
