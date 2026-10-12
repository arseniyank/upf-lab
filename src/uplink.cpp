#include "uplink.h"

#include <arpa/inet.h>       // htons, ntohs, ntohl
#include <linux/if_ether.h>  // ETH_P_IP
#include <sys/socket.h>      // send

#include <cstdio>   // printf, perror
#include <cstring>  // memcpy

#include "config.h"      // kUpfMac, kBroadcastMac, kBufSize
#include "net_utils.h"   // ipFromBytes, printIp
#include "protocols.h"   // EthHeader, Ipv4Header, GtpuHeader, kGtpuGpdu, kGtpuVersionPt, kGtpuOptionalMask

void handleGtpuFromGnb(int rawFd, const uint8_t* data, std::size_t len, const UeStorage& ues) {
    if (len < sizeof(GtpuHeader)) {
        std::printf("UL: short datagram (%zu bytes), drop\n", len);
        return;
    }
    GtpuHeader gtp;
    std::memcpy(&gtp, data, sizeof(gtp));

    if ((gtp.flags & 0xF0) != kGtpuVersionPt) {
        std::printf("UL: bad GTP version/PT (flags=0x%02x), drop\n", gtp.flags);
        return;
    }
    if ((gtp.flags & kGtpuOptionalMask) != 0) {
        std::printf("UL: optional GTP fields (flags=0x%02x) not supported, drop\n", gtp.flags);
        return;
    }

    if (gtp.type != kGtpuGpdu) {
        std::printf("UL: GTP message type %u not supported, drop\n", gtp.type);
        return;
    }

    const std::size_t payloadLen = ntohs(gtp.length);
    if (payloadLen != len - sizeof(GtpuHeader)) {
        std::printf("UL: GTP length %zu does not match datagram (%zu bytes), drop\n", payloadLen, len);
        return;
    }

    const uint32_t teid = ntohl(gtp.teid);
    const Ue* ue = ues.findByTeidUl(teid);
    if (ue == nullptr) {
        std::printf("UL: unknown teid_ul=%u, drop\n", teid);
        return;
    }

    const uint8_t* inner = data + sizeof(GtpuHeader);
    if (payloadLen < sizeof(Ipv4Header)) {
        std::printf("UL: inner packet too short (%zu), drop\n", payloadLen);
        return;
    }
    Ipv4Header ip;
    std::memcpy(&ip, inner, sizeof(ip));

    const unsigned version = ip.verIhl >> 4;
    const std::size_t ihl = static_cast<std::size_t>(ip.verIhl & 0x0F) * 4;
    if (version != 4 || ihl < sizeof(Ipv4Header)) {
        std::printf("UL: inner bad version/IHL (ver=%u ihl=%zu), drop\n", version, ihl);
        return;
    }
    const std::size_t totalLen = ntohs(ip.totalLen);
    if (totalLen < ihl || totalLen > payloadLen) {
        std::printf("UL: inner bad total length (%zu, payload %zu), drop\n", totalLen, payloadLen);
        return;
    }

    if (ipFromBytes(ip.srcIp) != ue->ip) {
        std::printf("UL: spoofed source ");
        printIp(ip.srcIp);
        std::printf(" for teid_ul=%u, drop\n", teid);
        return;
    }

    uint8_t out[sizeof(EthHeader) + kBufSize];
    EthHeader eth{};
    std::memcpy(eth.dst, kBroadcastMac, 6);
    std::memcpy(eth.src, kUpfMac, 6);
    eth.etherType = htons(ETH_P_IP);
    std::memcpy(out, &eth, sizeof(eth));
    std::memcpy(out + sizeof(eth), inner, totalLen);

    const std::size_t outLen = sizeof(eth) + totalLen;
    ssize_t sent = send(rawFd, out, outLen, 0);
    std::printf("UL: ");
    printIp(ip.srcIp);
    std::printf(" -> ");
    printIp(ip.dstIp);
    std::printf(" ip_len=%zu teid_ul=%u", totalLen, teid);
    if (sent < 0) {
        std::printf(" -> send failed\n");
        std::perror("send");
        return;
    }
    std::printf(" -> sent %zd bytes to DN\n", sent);
}
