#ifndef __IOHANDLER_H
#define __IOHANDLER_H

#include <sys/epoll.h>
#include <sys/socket.h>

#include <cstring>
#include <cstdio>
#include <mutex>
#include <memory>
#include <thread>
#include <sstream>

#include "kingpin/core/Exception.h"

using namespace std;

const int MAX_SIZE = 1024;

// IOHandler --> thread

template <typename _ThreadShareData>
class IOHandler {
public:
    int _epfd = ::epoll_create(1);
    struct epoll_event _evs[MAX_SIZE];
    shared_ptr<thread> _t;
    _ThreadShareData *_tsd_ptr;

    IOHandler(const IOHandler &) = delete;
    IOHandler &operator=(const IOHandler &) = delete;

    IOHandler(_ThreadShareData *tsd_ptr) : _tsd_ptr(tsd_ptr) {}

    void join() {
        _t->join();
    }

    void RegisterFd(int fd, uint32_t events) {
        struct epoll_event ev;
        ev.data.fd = fd;
        ev.events = events;
        if (::epoll_ctl(_epfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
            stringstream ss;
            ss << "register fd " << fd << " error";
            ::perror(ss.str().c_str());
            throw NonFatalException(ss.str().c_str());
        }
    }

    void RemoveFd(int fd) {
        if (::epoll_ctl(_epfd, EPOLL_CTL_DEL, fd, NULL) == -1) {
            stringstream ss;
            ss << "remove fd " << fd << " error";
            ::perror(ss.str().c_str());
            throw NonFatalException(ss.str().c_str());
        }
    }

    virtual void run() = 0;

    virtual void onReadable(int conn, uint32_t events) = 0;

    virtual void onWritable(int conn, uint32_t events) = 0;
};

#endif