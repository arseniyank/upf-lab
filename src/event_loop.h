#pragma once

#include <functional>     // function
#include <unordered_map>  // unordered_map

#include "fd.h"  // Fd

class EventLoop {
public:
    bool init();
    bool add(int fd, std::function<void()> onReadable);
    void run();

private:
    Fd epfd_;
    std::unordered_map<int, std::function<void()>> handlers_;
};
