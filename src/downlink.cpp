#include "downlink.h"

#include <arpa/inet.h>   // htons, htonl, ntohs
#include <sys/socket.h>  // sendto

#include <cstdio>   // printf, perror
#include <cstring>  // memcpy, memcmp

#include "config.h"      // kUpfMac, kBufSize
#include "net_utils.h"   // ipFromBytes, printIp
#include "protocols.h"   // EthHeader, Ipv4Header, GtpuHeader, kGtpuFlags, kGtpuGpdu

void handleIpv4FromDn(int gtpFd, const sockaddr_in& gnbAddr, const uint8_t* frame,
                      std::size_t len, const UeStorage& ues) {
    if (len < sizeof(EthHeader) + sizeof(Ipv4Header)) {
        std::printf("IPv4: short frame (%zu bytes), drop\n", len);
        return;
    }

    EthHeader eth;
    Ipv4Header ip;
    std::memcpy(&eth, frame, sizeof(eth));
    std::memcpy(&ip, frame + sizeof(eth), sizeof(ip));

    if (std::memcmp(eth.dst, kUpfMac, 6) != 0) {
        return;
    }

    const unsigned version = ip.verIhl >> 4;
    const std::size_t ihl = static_cast<std::size_t>(ip.verIhl & 0x0F) * 4;
    if (version != 4 || ihl < sizeof(Ipv4Header)) {
        std::printf("IPv4: bad version/IHL (ver=%u ihl=%zu), drop\n", version, ihl);
        return;
    }

    const std::size_t totalLen = ntohs(ip.totalLen);
    if (totalLen < ihl || sizeof(EthHeader) + totalLen > len) {
        std::printf("IPv4: bad total length (%zu, frame %zu), drop\n", totalLen, len);
        return;
    }

    const Ue* ue = ues.findByIp(ipFromBytes(ip.dstIp));
    if (ue == nullptr) {
        std::printf("DL: no UE for ");
        printIp(ip.dstIp);
        std::printf(", drop\n");
        return;
    }

    uint8_t out[sizeof(GtpuHeader) + kBufSize];
    GtpuHeader gtp{};
    gtp.flags = kGtpuFlags;
    gtp.type = kGtpuGpdu;
    gtp.length = htons(static_cast<uint16_t>(totalLen));
    gtp.teid = htonl(ue->teidDl);
    std::memcpy(out, &gtp, sizeof(gtp));
    std::memcpy(out + sizeof(gtp), frame + sizeof(eth), totalLen);

    const std::size_t outLen = sizeof(gtp) + totalLen;
    ssize_t sent = sendto(gtpFd, out, outLen, 0, reinterpret_cast<const sockaddr*>(&gnbAddr),
                          sizeof(gnbAddr));
    std::printf("DL: ");
    printIp(ip.srcIp);
    std::printf(" -> ");
    printIp(ip.dstIp);
    std::printf(" ip_len=%zu teid_dl=%u", totalLen, ue->teidDl);
    if (sent < 0) {
        std::printf(" -> sendto failed\n");
        std::perror("sendto");
        return;
    }
    std::printf(" -> sent %zd bytes to gNB\n", sent);
}
