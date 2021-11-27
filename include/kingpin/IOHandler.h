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
#include <chrono>

#include "kingpin/Exception.h"
#include "kingpin/Logger.h"
#include "kingpin/Utils.h"

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

    virtual void onReadable(int conn, uint32_t events) {}

    virtual void onWritable(int conn, uint32_t events) {}

    virtual void onPassivelyClosed(int conn) {}

    virtual ~IOHandler() {}
};



template <typename _ThreadSharedData>
class IOHandlerForClient : public IOHandler<_ThreadSharedData> {
public:
    IOHandlerForClient(const IOHandlerForClient &) = delete;
    IOHandlerForClient &operator=(const IOHandlerForClient &) = delete;

    IOHandlerForClient(_ThreadSharedData *tsd_ptr) : IOHandler<_ThreadSharedData>(tsd_ptr) {}

    virtual void onInit() {}

    void _run() {
        INFO << "thread start" << END;
        while (true) {
            int timeout = 10;

            if (this->_tsd_ptr->_m.try_lock()) {
                INFO << "thread get lock" << END;
                onInit();
                this->_tsd_ptr->_m.unlock();
                INFO << "thread release lock" << END;
                timeout = -1;
            }

            int num = epoll_wait(this->_epfd, this->_evs, MAX_SIZE, timeout);

            for (int i = 0; i < num; ++i) {
                int fd = this->_evs[i].data.fd;
                uint32_t events = this->_evs[i].events;
                if (events & EPOLLIN) {
                    INFO << "conn " << fd << " readable" << END;
                    this->onReadable(fd, events);
                }
                if (events & EPOLLOUT) {
                    INFO << "conn " << fd << " writable" << END;
                    this->onWritable(fd, events);
                }
            }
        }
    }

    void run() {
        this->_t = make_shared<thread>(&IOHandlerForClient::_run, this);
    }

    virtual ~IOHandlerForClient() {}
};



template <typename _ThreadSharedData>
class IOHandlerForServer : public IOHandler<_ThreadSharedData> {
public:
    IOHandlerForServer(const IOHandlerForServer &) = delete;
    IOHandlerForServer &operator=(const IOHandlerForServer &) = delete;

    IOHandlerForServer(_ThreadSharedData *tsd_ptr) : IOHandler<_ThreadSharedData>(tsd_ptr) {}

    virtual void onConnect(int conn) {}

    void _run() {
        INFO << "thread start" << END;
        while (true) {
            int timeout = 5;

            if (this->_tsd_ptr->_listenfd_lock.try_lock()) {
                timeout = -1;
                INFO << "thread get lock" << END;
                this->RegisterFd(this->_tsd_ptr->_listenfd, EPOLLIN);
            }

            int num = epoll_wait(this->_epfd, this->_evs, MAX_SIZE, timeout);

            for (int i = 0; i < num; ++i) {
                int fd = this->_evs[i].data.fd;
                uint32_t events = this->_evs[i].events;
                if (fd == this->_tsd_ptr->_listenfd) {
                    int conn = ::accept4(fd, NULL, NULL, SOCK_NONBLOCK);
                    if (conn < 0) {
                        fatalError("syscall accept4 error");
                    }
                    INFO << "new connection " << conn << " accepted" << END;
                    this->RemoveFd(fd);
                    this->_tsd_ptr->_listenfd_lock.unlock();
                    INFO << "thread release lock" << END;
                    this->onConnect(conn);
                    this_thread::sleep_for(chrono::milliseconds(1));
                } else {
                    if (events & EPOLLIN) {
                        INFO << "conn " << fd << " readable" << END;
                        this->onReadable(fd, events);
                    }
                    if (events & EPOLLOUT) {
                        INFO << "conn " << fd << " writable" << END;
                        this->onWritable(fd, events);
                    }
                }
            }
        }
    }

    void run() {
        this->_t = make_shared<thread>(&IOHandlerForServer::_run, this);
    }

    virtual ~IOHandlerForServer() {}
};

#endif
