#pragma once

#include <cstdint>  // uint8_t, uint16_t, uint32_t

struct __attribute__((packed)) EthHeader {
    uint8_t dst[6];
    uint8_t src[6];
    uint16_t etherType;
};

struct __attribute__((packed)) ArpPacket {
    uint16_t hwType;
    uint16_t protoType;
    uint8_t hwLen;
    uint8_t protoLen;
    uint16_t opcode;
    uint8_t senderMac[6];
    uint8_t senderIp[4];
    uint8_t targetMac[6];
    uint8_t targetIp[4];
};

struct __attribute__((packed)) Ipv4Header {
    uint8_t verIhl;
    uint8_t tos;
    uint16_t totalLen;
    uint16_t id;
    uint16_t fragOff;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t checksum;
    uint8_t srcIp[4];
    uint8_t dstIp[4];
};

struct __attribute__((packed)) GtpuHeader {
    uint8_t flags;
    uint8_t type;
    uint16_t length;
    uint32_t teid;
};

static_assert(sizeof(EthHeader) == 14, "EthHeader must be 14 bytes");
static_assert(sizeof(ArpPacket) == 28, "ArpPacket must be 28 bytes");
static_assert(sizeof(Ipv4Header) == 20, "Ipv4Header must be 20 bytes");
static_assert(sizeof(GtpuHeader) == 8, "GtpuHeader must be 8 bytes");

inline constexpr uint8_t kGtpuFlags = 0x30;
inline constexpr uint8_t kGtpuGpdu = 0xff;
inline constexpr uint8_t kGtpuVersionPt = 0x30;
inline constexpr uint8_t kGtpuOptionalMask = 0x07;
