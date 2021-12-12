#ifndef __IOHANDLER_H_de3094e9a992_
#define __IOHANDLER_H_de3094e9a992_

#include <sys/epoll.h>
#include <sys/socket.h>

#include <cstring>
#include <cstdio>
#include <mutex>
#include <memory>
#include <thread>
#include <sstream>
#include <chrono>
#include <unistd.h>

#include "kingpin/Exception.h"
#include "kingpin/AsyncLogger.h"
#include "kingpin/Utils.h"

using namespace std;

namespace kingpin {

// IOHandler --> thread

template <typename _TPSharedData>
class IOHandler {
public:
    static const int _default_epoll_timeout;
    static const int _max_client_per_thr;

    int _epfd = ::epoll_create(1);
    struct epoll_event _evs[_max_client_per_thr];
    shared_ptr<thread> _t;
    _TPSharedData *_tsd_ptr;
    int _fd_num = 0;

    explicit IOHandler(_TPSharedData *tsd_ptr) : _tsd_ptr(tsd_ptr) {}
    IOHandler(const IOHandler &) = delete;
    IOHandler &operator=(const IOHandler &) = delete;
    virtual ~IOHandler() {}

    virtual void onMessage(int conn) {}
    virtual void onWriteComplete(int conn) {}
    virtual void onPassivelyClosed(int conn) {}

    virtual void run() = 0;

    void join() { _t->join(); }

    void registerFd(int fd, uint32_t events) {
        if (_fd_num == _max_client_per_thr) throw FatalException("too many connections!");
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

    void removeFd(int fd) {
        _fd_num--;
        INFO << "remove fd " << fd << END;
        if (::epoll_ctl(_epfd, EPOLL_CTL_DEL, fd, NULL) == -1) {
            stringstream ss;
            ss << "remove fd " << fd << " error";
            fatalError(ss.str().c_str());
        }
    }

    void createBuffer(int conn) {
        unique_lock<mutex>(this->_tsd_ptr->_rbm);
        unique_lock<mutex>(this->_tsd_ptr->_wbm);
        this->_tsd_ptr->_rbh[conn] = shared_ptr<Buffer>(new Buffer());
        this->_tsd_ptr->_wbh[conn] = shared_ptr<Buffer>(new Buffer());
    }

    void destoryBuffer(int conn) {
        unique_lock<mutex>(this->_tsd_ptr->_rbm);
        unique_lock<mutex>(this->_tsd_ptr->_wbm);
        this->_tsd_ptr->_rbh.erase(conn);
        this->_tsd_ptr->_wbh.erase(conn);
    }

    Buffer *getReadBuffer(int conn) { return _tsd_ptr->_rbh[conn].get(); }

    Buffer *getWriteBuffer(int conn) { return _tsd_ptr->_wbh[conn].get(); }

    mutex &getRBMutex() { return _tsd_ptr->_rbm; }

    mutex &getWBMutex() { return _tsd_ptr->_wbm; }

    // This function would be executed by one thread serially
    // with onMessage, so NO worry about write buffer's parallel
    // write. And conn be registered EPOLLIN while write is not
    // completed.
    void writeToBuffer(int conn) {
        Buffer *wb;
        try {
            wb = this->_tsd_ptr->_wbh[conn].get();
            wb->writeNioFromBufferTillBlock(conn);
            if (wb->writeComplete() ) {
                wb->clear();
                onWriteComplete(conn); }
            else {
                removeFd(conn);
                registerFd(conn, EPOLLOUT);
            }
        } catch(const FdClosedException &e) {
            INFO << e << END;
            onPassivelyClosed(conn);
            destoryBuffer(conn);
            ::close(conn);
        }
    }

    void onWritable(int conn) {
        Buffer *wb;
        try {
            wb = this->_tsd_ptr->_wbh[conn].get();
            wb->writeNioFromBufferTillBlock(conn);
            if (wb->writeComplete() ) {
                wb->clear();
                onWriteComplete(conn);
                registerFd(conn, EPOLLIN);
            }
        } catch(const FdClosedException &e) {
            INFO << e << END;
            onPassivelyClosed(conn);
            destoryBuffer(conn);
            ::close(conn);
        }

    }

    void onReadable(int conn) {
        Buffer *rb;
        try {
            rb = this->_tsd_ptr->_rbh[conn].get();
            rb->readNioToBufferTillBlock(conn);
            onMessage(conn);
        } catch(const NonFatalException &e) {
            // opposite end may collapse or close the conn
            INFO << e << END;
            onPassivelyClosed(conn);
            destoryBuffer(conn);
            ::close(conn);
        }
    }
};



template <typename _TPSharedData>
class IOHandlerForClient : public IOHandler<_TPSharedData> {
public:
    IOHandlerForClient(const IOHandlerForClient &) = delete;
    IOHandlerForClient &operator=(const IOHandlerForClient &) = delete;
    IOHandlerForClient(_TPSharedData *tsd_ptr) : IOHandler<_TPSharedData>(tsd_ptr) {}
    virtual ~IOHandlerForClient() {}

    virtual void onInit() {}    // No lock hold

    void run() { this->_t = make_shared<thread>(&IOHandlerForClient::_run, this); }

    void _run() {
        INFO << "thread start" << END;
        while (true) {
            onInit();
            int num = epoll_wait(this->_epfd, this->_evs,
                this->_max_client_per_thr, this->_default_epoll_timeout);
            for (int i = 0; i < num; ++i) {
                int fd = this->_evs[i].data.fd;
                uint32_t events = this->_evs[i].events;
                if (events & EPOLLIN) {
                    INFO << "conn " << fd << " readable" << END;
                    this->onReadable(fd);
                }
                if (events & EPOLLOUT) {
                    INFO << "conn " << fd << " writable" << END;
                    this->onWritable(fd);
                }
            }
        }
    }
};



template <typename _TPSharedData>
class IOHandlerForServer : public IOHandler<_TPSharedData> {
public:
    static const int _conn_delay;

    IOHandlerForServer(const IOHandlerForServer &) = delete;
    IOHandlerForServer &operator=(const IOHandlerForServer &) = delete;
    IOHandlerForServer(_TPSharedData *tsd_ptr) : IOHandler<_TPSharedData>(tsd_ptr) {}
    virtual ~IOHandlerForServer() {}

    virtual void onConnect(int conn) {}     // No lock hold

    void run() { this->_t = make_shared<thread>(&IOHandlerForServer::_run, this); }

    void _run() {
        INFO << "thread start" << END;
        while (true) {
            int timeout = this->_default_epoll_timeout;
            if (this->_tsd_ptr->_listenfd_lock.try_lock()) {
                timeout = -1;
                INFO << "thread get lock" << END;
                this->registerFd(this->_tsd_ptr->_listenfd, EPOLLIN);
            }
            int num = epoll_wait(this->_epfd, this->_evs,
                this->_max_client_per_thr, timeout);
            for (int i = 0; i < num; ++i) {
                int fd = this->_evs[i].data.fd;
                uint32_t events = this->_evs[i].events;
                if (fd == this->_tsd_ptr->_listenfd) {
                    int conn = ::accept4(fd, NULL, NULL, SOCK_NONBLOCK);
                    if (conn < 0) { fatalError("syscall accept4 error"); }
                    INFO << "new connection " << conn << " accepted" << END;
                    this->removeFd(fd);
                    this->_tsd_ptr->_listenfd_lock.unlock();
                    INFO << "thread release lock" << END;
                    this->createBuffer(conn);
                    this->registerFd(conn, EPOLLIN);
                    this->onConnect(conn);
                    this_thread::sleep_for(chrono::microseconds(_conn_delay));
                } else {
                    if (events & EPOLLIN) {
                        INFO << "conn " << fd << " readable" << END;
                        this->onReadable(fd);
                    }
                    if (events & EPOLLOUT) {
                        INFO << "conn " << fd << " writable" << END;
                        this->onWritable(fd);
                    }
                }
            }
        }
    }
};

template <typename _TPSharedData>
const int IOHandler<_TPSharedData>::_default_epoll_timeout = 1;    // milliseconds

template <typename _TPSharedData>
const int IOHandler<_TPSharedData>::_max_client_per_thr = 2048;

template <typename _TPSharedData>
const int IOHandlerForServer<_TPSharedData>::_conn_delay = 100;     // microseconds

}

#endif
