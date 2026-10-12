#include "sockets.h"

#include <arpa/inet.h>        // htons
#include <fcntl.h>            // fcntl, F_GETFL, F_SETFL, O_NONBLOCK
#include <linux/if_ether.h>   // ETH_P_ALL
#include <linux/if_packet.h>  // sockaddr_ll
#include <net/if.h>           // if_nametoindex
#include <sys/socket.h>       // socket, bind, AF_PACKET, AF_INET, SOCK_RAW, SOCK_DGRAM

#include <cstdio>  // printf, fprintf, perror, stderr

#include "config.h"     // kUpfAnIp, kGtpuPort
#include "net_utils.h"  // makeSockaddr

Fd openRawSocket(const char* ifName) {
    Fd fd(socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL)));
    if (!fd.valid()) {
        std::perror("socket(AF_PACKET)");
        return Fd();
    }

    unsigned int ifindex = if_nametoindex(ifName);
    if (ifindex == 0) {
        std::perror("if_nametoindex");
        return Fd();
    }

    sockaddr_ll addr{};
    addr.sll_family = AF_PACKET;
    addr.sll_protocol = htons(ETH_P_ALL);
    addr.sll_ifindex = static_cast<int>(ifindex);
    if (bind(fd.get(), reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        std::perror("bind(AF_PACKET)");
        return Fd();
    }

    std::printf("raw socket: %s (ifindex=%u)\n", ifName, ifindex);
    return fd;
}

Fd openGtpuSocket() {
    Fd fd(socket(AF_INET, SOCK_DGRAM, 0));
    if (!fd.valid()) {
        std::perror("socket(AF_INET)");
        return Fd();
    }

    sockaddr_in addr;
    if (!makeSockaddr(kUpfAnIp, kGtpuPort, addr)) {
        std::fprintf(stderr, "bad address %s\n", kUpfAnIp);
        return Fd();
    }
    if (bind(fd.get(), reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        std::perror("bind(AF_INET)");
        return Fd();
    }

    std::printf("udp socket: %s:%u\n", kUpfAnIp, static_cast<unsigned>(kGtpuPort));
    return fd;
}

bool setNonBlocking(int fd) {
    const int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) {
        return false;
    }
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK) == 0;
}
