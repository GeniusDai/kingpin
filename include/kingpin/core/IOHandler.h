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
#include "kingpin/core/Logger.h"
#include "kingpin/utils/Utils.h"

using namespace std;

const int MAX_SIZE = 1024;

// IOHandler --> thread

template <typename _ThreadSharedData>
class IOHandler {
public:
    int _epfd = ::epoll_create(1);
    struct epoll_event _evs[MAX_SIZE];
    shared_ptr<thread> _t;
    _ThreadSharedData *_tsd_ptr;
    int _fd_num = 0;

    IOHandler(const IOHandler &) = delete;
    IOHandler &operator=(const IOHandler &) = delete;

    IOHandler(_ThreadSharedData *tsd_ptr) : _tsd_ptr(tsd_ptr) {}

    void join() {
        _t->join();
    }

    void RegisterFd(int fd, uint32_t events) {
        if (_fd_num == MAX_SIZE) throw FatalException("too many connections!");
        _fd_num++;
        INFO << "register fd " << fd << END;
        struct epoll_event ev;
        ev.data.fd = fd;
        ev.events = events;
        if (::epoll_ctl(_epfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
            stringstream ss;
            ss << "register fd " << fd << " error";
            fatalError(ss.str().c_str());
        }
    }

    void RemoveFd(int fd) {
        _fd_num--;
        INFO << "remove fd " << fd << END;
        if (::epoll_ctl(_epfd, EPOLL_CTL_DEL, fd, NULL) == -1) {
            stringstream ss;
            ss << "remove fd " << fd << " error";
            fatalError(ss.str().c_str());
        }
    }

    virtual void run() = 0;

    virtual void onReadable(int conn, uint32_t events) = 0;

    virtual void onWritable(int conn, uint32_t events) {}

    virtual void onPassivelyClosed(int conn) {}

    virtual ~IOHandler() {}
};

#endif
