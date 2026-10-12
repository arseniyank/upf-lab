#include "net_utils.h"

#include <arpa/inet.h>  // htons, inet_pton

#include <cstdio>  // printf

uint32_t ipFromBytes(const uint8_t* b) {
    return (static_cast<uint32_t>(b[0]) << 24) | (static_cast<uint32_t>(b[1]) << 16) |
           (static_cast<uint32_t>(b[2]) << 8) | static_cast<uint32_t>(b[3]);
}

void printMac(const uint8_t* m) {
    std::printf("%02x:%02x:%02x:%02x:%02x:%02x", m[0], m[1], m[2], m[3], m[4], m[5]);
}

void printIp(const uint8_t* ip) {
    std::printf("%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
}

bool makeSockaddr(const char* ip, uint16_t port, sockaddr_in& out) {
    out = sockaddr_in{};
    out.sin_family = AF_INET;
    out.sin_port = htons(port);
    return inet_pton(AF_INET, ip, &out.sin_addr) == 1;
}
