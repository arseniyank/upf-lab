#pragma once

#include <netinet/in.h>  // sockaddr_in

#include "event_loop.h"  // EventLoop
#include "fd.h"          // Fd
#include "ue_storage.h"  // UeStorage

class Upf {
public:
    bool init();
    void run();

private:
    void onRawReadable();
    void onGtpReadable();

    UeStorage ues_;
    sockaddr_in gnbAddr_{};
    Fd rawSock_;
    Fd gtpSock_;
    EventLoop loop_;
};
