#ifndef __IOHANDLER_H_de3094e9a992_
#define __IOHANDLER_H_de3094e9a992_

#include <sys/epoll.h>
#include <sys/socket.h>
#include <cassert>
#include <cstring>
#include <cstdio>
#include <mutex>
#include <memory>
#include <thread>
#include <sstream>
#include <chrono>
#include <unistd.h>
#include <unordered_map>

#include "kingpin/Exception.h"
#include "kingpin/AsyncLogger.h"
#include "kingpin/Utils.h"

using namespace std;

namespace kingpin {

// IOHandler --> thread

template <typename _TPSharedData>
class IOHandler {
public:
    static const int _max_client_per_thr;
    static const int _default_epoll_timeout;

    int _epfd = ::epoll_create(1);
    struct epoll_event _evs[_max_client_per_thr];
    shared_ptr<thread> _t;
    _TPSharedData *_tsd;
    int _fd_num = 0;
    int _epoll_timeout = _default_epoll_timeout;

    unordered_map<int, shared_ptr<Buffer> > _rbh;
    unordered_map<int, shared_ptr<Buffer> > _wbh;

    explicit IOHandler(_TPSharedData *tsd) : _tsd(tsd) {}
    IOHandler(const IOHandler &) = delete;
    IOHandler &operator=(const IOHandler &) = delete;
    virtual ~IOHandler() {}

    virtual void onEpollLoop() {}
    virtual void onConnect(int conn) {}
    virtual void onMessage(int conn) {}
    virtual void onWriteComplete(int conn) {}
    virtual void onPassivelyClosed(int conn) {}

    virtual void run() = 0;

    void join() { _t->join(); }

    void registerFd(int fd, uint32_t events) {
        if (_fd_num == _max_client_per_thr) throw FatalException("too many connections!");
        INFO << "register fd " << fd << END;
        epollRegister(_epfd, fd, events);
    }

    void removeFd(int fd) {
        INFO << "remove fd " << fd << END;
        epollRemove(_epfd, fd);
    }

    void createBuffer(int conn) {
        _rbh[conn] = shared_ptr<Buffer>(new Buffer());
        _wbh[conn] = shared_ptr<Buffer>(new Buffer());
    }

    void destoryBuffer(int conn) {
        _rbh.erase(conn);
        _wbh.erase(conn);
    }

    // This function would be executed by one thread serially
    // with onMessage, so NO worry about write buffer's parallel
    // write. And conn be registered EPOLLIN while write is not
    // completed.
    void writeToBuffer(int conn) {
        Buffer *wb;
        try {
            wb = _wbh[conn].get();
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
            wb = _wbh[conn].get();
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
            rb = _rbh[conn].get();
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

    void processEvent(struct epoll_event &event) {
        int fd = event.data.fd;
        uint32_t events = event.events;
        if (events & EPOLLIN) {
            INFO << "conn " << fd << " readable" << END;
            this->onReadable(fd);
        }
        if (events & EPOLLOUT) {
            INFO << "conn " << fd << " writable" << END;
            this->onWritable(fd);
        }
    }
};



template <typename _TPSharedData>
class IOHandlerForClient : public IOHandler<_TPSharedData> {
public:
    int _epfd_conn = ::epoll_create(1);     // for nonblock connect
    int _epoll_timeout_conn = this->_default_epoll_timeout;

    unordered_map<int, string> _conn_init_message;
    unordered_map<int, pair<string, int> > _conn_info;

    IOHandlerForClient(const IOHandlerForClient &) = delete;
    IOHandlerForClient &operator=(const IOHandlerForClient &) = delete;
    IOHandlerForClient(_TPSharedData *tsd) : IOHandler<_TPSharedData>(tsd) {}
    virtual ~IOHandlerForClient() {}

    virtual void onConnectFailed(int conn) {}   // only client

    void run() { this->_t = make_shared<thread>(&IOHandlerForClient::_run, this); }

    void _get_from_pool() {
        assert(this->_tsd->_pool.size() > 0);
        ++(this->_fd_num);
        auto iter = this->_tsd->_pool.begin();
        int sock = connectIp(iter->first.first.c_str(), iter->first.second, 0);
        INFO << "connecting to " << iter->first.first.c_str()
            << ":" <<  iter->first.second << END;
        epollRegister(_epfd_conn, sock, EPOLLOUT);
        _conn_init_message[sock] = iter->second;
        _conn_info[sock] = iter->first;
        this->_tsd->_pool.erase(iter);
    }

    void _check_connect() {
        int num = ::epoll_wait(_epfd_conn, this->_evs,
            this->_max_client_per_thr, _epoll_timeout_conn);
        for (int i = 0; i < num; ++i) {
            uint32_t events = this->_evs[i].events;
            int fd = this->_evs[i].data.fd;
            assert(events & EPOLLOUT);
            epollRemove(_epfd_conn, fd);
            int optval;
            socklen_t len = sizeof(optval);
            if (::getsockopt(fd, SOL_SOCKET, SO_ERROR, &optval, &len) < 0) {
                fatalError("syscall getsockopt error");
            }
            if (0 == optval) {
                INFO << "connect succeed with conn " << fd << END;
                int size = _conn_init_message[fd].size();
                if (size) { ::write(fd, _conn_init_message[fd].c_str(), size); }
                this->createBuffer(fd);
                this->onConnect(fd);
                this->registerFd(fd, EPOLLIN);
            } else {
                INFO << "connect failed with conn " << fd << END;
                this->onConnectFailed(fd);
                ::close(fd);
            }
        }
    }

    void _run() {
        INFO << "thread start" << END;
        while (true) {
            this->onEpollLoop();
            if (0 == this->_fd_num) {
                unique_lock<mutex> m(this->_tsd->_pool_lock);
                this->_tsd->_cv.wait(m, [this]()->bool{
                    return this->_tsd->_pool.size() != 0; });
                INFO << "thread get lock" << END;
                _get_from_pool();
            } else if (this->_tsd->_pool.size() != 0 &&
                    this->_tsd->_pool_lock.try_lock()) {
                INFO << "thread get lock" << END;
                _get_from_pool();
                this->_tsd->_pool_lock.unlock();
            }
            assert(this->_fd_num > 0);
            _check_connect();
            int num = ::epoll_wait(this->_epfd, this->_evs,
                this->_max_client_per_thr, this->_epoll_timeout);
            for (int i = 0; i < num; ++i) { this->processEvent(this->_evs[i]); }
        }
    }
};



template <typename _TPSharedData>
class IOHandlerForServer : public IOHandler<_TPSharedData> {
public:
    static const int _conn_delay;

    IOHandlerForServer(const IOHandlerForServer &) = delete;
    IOHandlerForServer &operator=(const IOHandlerForServer &) = delete;
    IOHandlerForServer(_TPSharedData *tsd) : IOHandler<_TPSharedData>(tsd) {}
    virtual ~IOHandlerForServer() {}

    void run() { this->_t = make_shared<thread>(&IOHandlerForServer::_run, this); }

    void _run() {
        INFO << "thread start" << END;
        while (true) {
            this->onEpollLoop();
            bool hold_listen_lock = false;
            if (0 == this->_fd_num) {
                this->_tsd->_listenfd_lock.lock();
                hold_listen_lock = true;
            }
            else if (this->_tsd->_listenfd_lock.try_lock()) {
                hold_listen_lock = true;
            }
            if (hold_listen_lock) {
                INFO << "thread get lock" << END;
                ++(this->_fd_num);
                this->registerFd(this->_tsd->_listenfd, EPOLLIN);
            }
            assert(this->_fd_num > 0);
            int num = epoll_wait(this->_epfd, this->_evs,
                this->_max_client_per_thr, this->_epoll_timeout);
            for (int i = 0; i < num; ++i) {
                int fd = this->_evs[i].data.fd;
                if (fd != this->_tsd->_listenfd) {
                    this->processEvent(this->_evs[i]);
                } else {
                    int conn = ::accept4(fd, NULL, NULL, SOCK_NONBLOCK);
                    if (conn < 0) { fatalError("syscall accept4 error"); }
                    INFO << "new connection " << conn << " accepted" << END;
                    this->removeFd(fd);
                    this->_tsd->_listenfd_lock.unlock();
                    INFO << "thread release lock" << END;
                    this->createBuffer(conn);
                    this->registerFd(conn, EPOLLIN);
                    this->onConnect(conn);
                    this_thread::sleep_for(chrono::microseconds(_conn_delay));
                }
            }
        }
    }
};

template <typename _TPSharedData>
const int IOHandler<_TPSharedData>::_max_client_per_thr = 2048;

template <typename _TPSharedData>
const int IOHandler<_TPSharedData>::_default_epoll_timeout = 1;    // milliseconds

template <typename _TPSharedData>
const int IOHandlerForServer<_TPSharedData>::_conn_delay = 100;             // microseconds

}

#endif
