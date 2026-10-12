#pragma once

#include <netinet/in.h>  // sockaddr_in

#include <cstddef>  // size_t
#include <cstdint>  // uint8_t

#include "ue_storage.h"  // UeStorage

void handleIpv4FromDn(int gtpFd, const sockaddr_in& gnbAddr, const uint8_t* frame,
                      std::size_t len, const UeStorage& ues);
