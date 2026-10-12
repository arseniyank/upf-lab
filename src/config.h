#pragma once

#include <cstddef>  // size_t
#include <cstdint>  // uint8_t, uint16_t, uint32_t

inline constexpr const char* kDnIfName = "upf_cn_veth";
inline constexpr const char* kUpfAnIp = "192.168.9.1";
inline constexpr const char* kGnbIp = "192.168.9.2";
inline constexpr uint16_t kGtpuPort = 2152;

inline constexpr std::size_t kBufSize = 2048;

inline constexpr uint8_t kUpfMac[6] = {0xbb, 0x00, 0x00, 0x00, 0x00, 0x01};
inline constexpr uint8_t kBroadcastMac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

inline constexpr uint32_t kUeNet = (192u << 24) | (168u << 16) | (7u << 8);
