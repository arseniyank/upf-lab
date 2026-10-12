#pragma once

#include <netinet/in.h>  // sockaddr_in

#include <cstdint>  // uint8_t, uint16_t, uint32_t

uint32_t ipFromBytes(const uint8_t* b);
void printMac(const uint8_t* m);
void printIp(const uint8_t* ip);
bool makeSockaddr(const char* ip, uint16_t port, sockaddr_in& out);
