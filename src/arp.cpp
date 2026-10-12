#include "arp.h"

#include <arpa/inet.h>       // htons, ntohs
#include <linux/if_ether.h>  // ETH_P_ARP, ETH_P_IP
#include <net/if_arp.h>      // ARPHRD_ETHER, ARPOP_REQUEST, ARPOP_REPLY
#include <sys/socket.h>      // send

#include <cstdio>   // printf, perror
#include <cstring>  // memcpy

#include "config.h"      // kUpfMac
#include "net_utils.h"   // ipFromBytes, printIp, printMac
#include "protocols.h"   // EthHeader, ArpPacket

void handleArp(int rawFd, const uint8_t* frame, std::size_t len, const UeStorage& ues) {
    if (len < sizeof(EthHeader) + sizeof(ArpPacket)) {
        std::printf("ARP: short frame (%zu bytes), drop\n", len);
        return;
    }

    EthHeader eth;
    ArpPacket arp;
    std::memcpy(&eth, frame, sizeof(eth));
    std::memcpy(&arp, frame + sizeof(eth), sizeof(arp));

    if (ntohs(arp.hwType) != ARPHRD_ETHER || ntohs(arp.protoType) != ETH_P_IP ||
        arp.hwLen != 6 || arp.protoLen != 4) {
        std::printf("ARP: unsupported hw/proto type, drop\n");
        return;
    }

    if (ntohs(arp.opcode) != ARPOP_REQUEST) {
        return;
    }

    std::printf("ARP request: who-has ");
    printIp(arp.targetIp);
    std::printf(" tell ");
    printIp(arp.senderIp);

    if (ues.findByIp(ipFromBytes(arp.targetIp)) == nullptr) {
        std::printf(" -> ignore (not a UE address)\n");
        return;
    }

    EthHeader outEth{};
    std::memcpy(outEth.dst, eth.src, 6);
    std::memcpy(outEth.src, kUpfMac, 6);
    outEth.etherType = htons(ETH_P_ARP);

    ArpPacket outArp{};
    outArp.hwType = htons(ARPHRD_ETHER);
    outArp.protoType = htons(ETH_P_IP);
    outArp.hwLen = 6;
    outArp.protoLen = 4;
    outArp.opcode = htons(ARPOP_REPLY);
    std::memcpy(outArp.senderMac, kUpfMac, 6);
    std::memcpy(outArp.senderIp, arp.targetIp, 4);
    std::memcpy(outArp.targetMac, arp.senderMac, 6);
    std::memcpy(outArp.targetIp, arp.senderIp, 4);

    uint8_t reply[sizeof(EthHeader) + sizeof(ArpPacket)];
    std::memcpy(reply, &outEth, sizeof(outEth));
    std::memcpy(reply + sizeof(outEth), &outArp, sizeof(outArp));

    if (send(rawFd, reply, sizeof(reply), 0) < 0) {
        std::printf(" -> send failed\n");
        std::perror("send");
        return;
    }
    std::printf(" -> reply sent: is-at ");
    printMac(kUpfMac);
    std::printf("\n");
}
