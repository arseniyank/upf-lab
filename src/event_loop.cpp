#include "event_loop.h"

#include <sys/epoll.h>  // epoll_create1, epoll_ctl, epoll_wait, epoll_event, EPOLLIN, EPOLL_CTL_ADD

#include <cerrno>   // errno, EINTR
#include <cstdio>   // perror
#include <utility>  // move

namespace {

constexpr int kMaxEvents = 8;

}

bool EventLoop::init() {
    epfd_ = Fd(epoll_create1(0));
    if (!epfd_.valid()) {
        std::perror("epoll_create1");
        return false;
    }
    return true;
}

bool EventLoop::add(int fd, std::function<void()> onReadable) {
    epoll_event ev{};
    ev.events = EPOLLIN;
    ev.data.fd = fd;
    if (epoll_ctl(epfd_.get(), EPOLL_CTL_ADD, fd, &ev) < 0) {
        std::perror("epoll_ctl(ADD)");
        return false;
    }
    handlers_[fd] = std::move(onReadable);
    return true;
}

void EventLoop::run() {
    epoll_event events[kMaxEvents];
    while (true) {
        const int n = epoll_wait(epfd_.get(), events, kMaxEvents, -1);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            std::perror("epoll_wait");
            return;
        }

        for (int i = 0; i < n; ++i) {
            auto it = handlers_.find(events[i].data.fd);
            if (it != handlers_.end()) {
                it->second();
            }
        }
    }
}
