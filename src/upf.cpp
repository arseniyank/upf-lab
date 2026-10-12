#include "upf.h"

#include <arpa/inet.h>        // ntohs
#include <linux/if_ether.h>   // ETH_P_ARP, ETH_P_IP
#include <linux/if_packet.h>  // sockaddr_ll, PACKET_OUTGOING
#include <sys/socket.h>       // recvfrom

#include <cerrno>   // errno, EINTR, EAGAIN, EWOULDBLOCK
#include <cstdio>   // printf, fprintf, perror, stderr
#include <cstring>  // memcpy

#include "arp.h"         // handleArp
#include "config.h"      // kDnIfName, kGnbIp, kGtpuPort, kBufSize
#include "downlink.h"    // handleIpv4FromDn
#include "net_utils.h"   // makeSockaddr
#include "protocols.h"   // EthHeader
#include "sockets.h"     // openRawSocket, openGtpuSocket, setNonBlocking
#include "uplink.h"      // handleGtpuFromGnb

bool Upf::init() {
    rawSock_ = openRawSocket(kDnIfName);
    if (!rawSock_.valid()) {
        return false;
    }
    gtpSock_ = openGtpuSocket();
    if (!gtpSock_.valid()) {
        return false;
    }
    if (!setNonBlocking(rawSock_.get()) || !setNonBlocking(gtpSock_.get())) {
        std::perror("fcntl(O_NONBLOCK)");
        return false;
    }

    if (!makeSockaddr(kGnbIp, kGtpuPort, gnbAddr_)) {
        std::fprintf(stderr, "bad address %s\n", kGnbIp);
        return false;
    }

    if (!loop_.init()) {
        return false;
    }
    if (!loop_.add(rawSock_.get(), [this] { onRawReadable(); }) ||
        !loop_.add(gtpSock_.get(), [this] { onGtpReadable(); })) {
        return false;
    }
    return true;
}

void Upf::run() {
    loop_.run();
}

void Upf::onRawReadable() {
    uint8_t buf[kBufSize];
    sockaddr_ll from{};
    socklen_t fromLen = sizeof(from);
    ssize_t n = recvfrom(rawSock_.get(), buf, sizeof(buf), 0,
                         reinterpret_cast<sockaddr*>(&from), &fromLen);
    if (n < 0) {
        if (errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK) {
            std::perror("recvfrom(raw)");
        }
        return;
    }

    if (from.sll_pkttype == PACKET_OUTGOING) {
        return;
    }

    const std::size_t len = static_cast<std::size_t>(n);
    if (len < sizeof(EthHeader)) {
        std::printf("short frame: %zu bytes, skip\n", len);
        return;
    }

    EthHeader eth;
    std::memcpy(&eth, buf, sizeof(eth));
    const uint16_t etherType = ntohs(eth.etherType);

    if (etherType == ETH_P_ARP) {
        handleArp(rawSock_.get(), buf, len, ues_);
    } else if (etherType == ETH_P_IP) {
        handleIpv4FromDn(gtpSock_.get(), gnbAddr_, buf, len, ues_);
    } else {
        std::printf("skip: ethertype=0x%04x len=%zu\n", etherType, len);
    }
}

void Upf::onGtpReadable() {
    uint8_t buf[kBufSize];
    sockaddr_in from{};
    socklen_t fromLen = sizeof(from);
    ssize_t n = recvfrom(gtpSock_.get(), buf, sizeof(buf), 0,
                         reinterpret_cast<sockaddr*>(&from), &fromLen);
    if (n < 0) {
        if (errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK) {
            std::perror("recvfrom(gtp)");
        }
        return;
    }

    if (from.sin_addr.s_addr != gnbAddr_.sin_addr.s_addr) {
        std::printf("UL: datagram from unexpected address, drop\n");
        return;
    }

    handleGtpuFromGnb(rawSock_.get(), buf, static_cast<std::size_t>(n), ues_);
}
