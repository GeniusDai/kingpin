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
    unique_ptr<struct epoll_event[]> __evs_ptr;
public:
    _TPSharedData *_tsd;
    int _epfd = ::epoll_create(1);
    struct epoll_event *_evs;
    unique_ptr<thread> _t;
    int _fd_num = 0;

    unordered_map<int, unique_ptr<Buffer> > _rbh;
    unordered_map<int, unique_ptr<Buffer> > _wbh;

    explicit IOHandler(_TPSharedData *tsd);
    IOHandler(const IOHandler &) = delete;
    IOHandler &operator=(const IOHandler &) = delete;
    virtual ~IOHandler();

    virtual void onEpollLoop();
    virtual void onConnect(int conn);
    virtual void onMessage(int conn);
    virtual void onWriteComplete(int conn);
    virtual void onPassivelyClosed(int conn);
    virtual void run() = 0;

    void join();
    void createBuffer(int conn);
    void destoryBuffer(int conn);
    void writeToBuffer(int conn);
    void onWritable(int conn);
    void onReadable(int conn);
    void processEvent(struct epoll_event &event);
};

template <typename _TPSharedData>
IOHandler<_TPSharedData>::IOHandler(_TPSharedData *tsd) : _tsd(tsd) {
    __evs_ptr = unique_ptr<epoll_event[]>(new epoll_event[_tsd->_max_client_per_thr]);
    _evs = __evs_ptr.get();
}

template <typename _TPSharedData>
IOHandler<_TPSharedData>::~IOHandler() {}

template <typename _TPSharedData>
void IOHandler<_TPSharedData>::onEpollLoop() {}

template <typename _TPSharedData>
void IOHandler<_TPSharedData>::onConnect(int conn) {}

template <typename _TPSharedData>
void IOHandler<_TPSharedData>::onMessage(int conn) {}

template <typename _TPSharedData>
void IOHandler<_TPSharedData>::onWriteComplete(int conn) {}

template <typename _TPSharedData>
void IOHandler<_TPSharedData>::onPassivelyClosed(int conn) {}

template <typename _TPSharedData>
void IOHandler<_TPSharedData>::join() { _t->join(); }

template <typename _TPSharedData>
void IOHandler<_TPSharedData>::createBuffer(int conn) {
    _rbh[conn] = unique_ptr<Buffer>(new Buffer());
    _wbh[conn] = unique_ptr<Buffer>(new Buffer());
}

template <typename _TPSharedData>
void IOHandler<_TPSharedData>::destoryBuffer(int conn) {
    _rbh.erase(conn);
    _wbh.erase(conn);
}

// This function would be executed by one thread serially
// with onMessage, so NO worry about write buffer's parallel
// write. And conn be registered EPOLLIN while write is not
// completed.
template <typename _TPSharedData>
void IOHandler<_TPSharedData>::writeToBuffer(int conn) {
    Buffer *wb;
    try {
        wb = _wbh[conn].get();
        wb->writeNioFromBufferTillBlock(conn);
        if (wb->writeComplete() ) {
            wb->clear();
            onWriteComplete(conn); }
        else {
            epollRemove(_epfd, conn);
            epollRegister(_epfd, conn, EPOLLOUT);
        }
    } catch(const FdClosedException &e) {
        INFO << e << END;
        onPassivelyClosed(conn);
        destoryBuffer(conn);
        ::close(conn);
    }
}


template <typename _TPSharedData>
void IOHandler<_TPSharedData>::onWritable(int conn) {
    Buffer *wb;
    try {
        wb = _wbh[conn].get();
        wb->writeNioFromBufferTillBlock(conn);
        if (wb->writeComplete() ) {
            wb->clear();
            onWriteComplete(conn);
            epollRegister(_epfd, conn, EPOLLIN);
        }
    } catch(const FdClosedException &e) {
        INFO << e << END;
        onPassivelyClosed(conn);
        destoryBuffer(conn);
        ::close(conn);
    }
}

template <typename _TPSharedData>
void IOHandler<_TPSharedData>::onReadable(int conn) {
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

template <typename _TPSharedData>
void IOHandler<_TPSharedData>::processEvent(struct epoll_event &event) {
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


template <typename _TPSharedData>
class IOHandlerForClient : public IOHandler<_TPSharedData> {
public:
    int _epfd_conn = ::epoll_create(1);     // for nonblock connect

    unordered_map<int, string> _conn_init_message;
    unordered_map<int, pair<string, int> > _conn_info;

    explicit IOHandlerForClient(_TPSharedData *tsd);
    IOHandlerForClient(const IOHandlerForClient &) = delete;
    IOHandlerForClient &operator=(const IOHandlerForClient &) = delete;
    virtual ~IOHandlerForClient() = 0;

    virtual void onConnectFailed(int conn);   // only client

    void run();
    void _get_from_pool(int batch);
    void _check_connect();
    void _run();
};


template <typename _TPSharedData>
IOHandlerForClient<_TPSharedData>::IOHandlerForClient(_TPSharedData *tsd)
    : IOHandler<_TPSharedData>(tsd) {}

template <typename _TPSharedData>
IOHandlerForClient<_TPSharedData>::~IOHandlerForClient() {}

template <typename _TPSharedData>
void IOHandlerForClient<_TPSharedData>::onConnectFailed(int conn) {}

template <typename _TPSharedData>
void IOHandlerForClient<_TPSharedData>::run() {
    this->_t = unique_ptr<thread>(new thread(&IOHandlerForClient::_run, this));
}

template <typename _TPSharedData>
void IOHandlerForClient<_TPSharedData>::_get_from_pool(int batch) {
    while(batch--) {
        if (this->_tsd->_pool.size() == 0) { break; }
        ++(this->_fd_num);
        tuple<string, int, string> t = this->_tsd->raw_get();
        int sock = connectIp(get<0>(t).c_str(), get<1>(t), 0);
        INFO << "connecting to " << get<0>(t).c_str() << ":" <<  get<1>(t) << END;
        epollRegister(_epfd_conn, sock, EPOLLOUT);
        _conn_init_message[sock] = get<2>(t);
        _conn_info[sock] = make_pair(get<0>(t), get<1>(t));
    }
}

template <typename _TPSharedData>
void IOHandlerForClient<_TPSharedData>::_check_connect() {
    int num = ::epoll_wait(_epfd_conn, this->_evs,
        this->_tsd->_max_client_per_thr, this->_tsd->_ep_timeout_conn);
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
            epollRegister(this->_epfd, fd, EPOLLIN);
        } else {
            INFO << "connect failed with conn " << fd << END;
            this->onConnectFailed(fd);
            ::close(fd);
        }
    }
}

template <typename _TPSharedData>
void IOHandlerForClient<_TPSharedData>::_run() {
    INFO << "thread start" << END;
    while (true) {
    try {
        this->onEpollLoop();
        if (0 == this->_fd_num) {
            unique_lock<mutex> m(this->_tsd->_pool_lock);
            this->_tsd->_cv.wait(m, [this]()->bool{
                return this->_tsd->_pool.size() != 0; });
            INFO << "thread get lock" << END;
            _get_from_pool(this->_tsd->_batch);
        } else if (this->_tsd->_pool.size() != 0 &&
                this->_tsd->_pool_lock.try_lock()) {
            INFO << "thread get lock" << END;
            _get_from_pool(this->_tsd->_batch);
            this->_tsd->_pool_lock.unlock();
        }
        assert(this->_fd_num > 0);
        _check_connect();
        int num = ::epoll_wait(this->_epfd, this->_evs,
            this->_tsd->_max_client_per_thr, this->_tsd->_ep_timeout);
        for (int i = 0; i < num; ++i) { this->processEvent(this->_evs[i]); }
    } catch (const exception &e) { INFO << e.what() << END; }
    }
}


template <typename _TPSharedData>
class IOHandlerForServer : public IOHandler<_TPSharedData> {
public:
    explicit IOHandlerForServer(_TPSharedData *tsd);
    IOHandlerForServer(const IOHandlerForServer &) = delete;
    IOHandlerForServer &operator=(const IOHandlerForServer &) = delete;
    virtual ~IOHandlerForServer() = 0;

    void run();
    void _run();
};

template <typename _TPSharedData>
IOHandlerForServer<_TPSharedData>::IOHandlerForServer(_TPSharedData *tsd)
    : IOHandler<_TPSharedData>(tsd) {}

template <typename _TPSharedData>
IOHandlerForServer<_TPSharedData>::~IOHandlerForServer() {}

template <typename _TPSharedData>
void IOHandlerForServer<_TPSharedData>::run() {
    this->_t = unique_ptr<thread>(new thread(&IOHandlerForServer::_run, this));
}

template <typename _TPSharedData>
void IOHandlerForServer<_TPSharedData>::_run() {
    INFO << "thread start" << END;
    while (true) {
    try {
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
            epollRegister(this->_epfd, this->_tsd->_listenfd, EPOLLIN);
        }
        assert(this->_fd_num > 0);
        int num = epoll_wait(this->_epfd, this->_evs,
            this->_tsd->_max_client_per_thr, this->_tsd->_ep_timeout);
        for (int i = 0; i < num; ++i) {
            int fd = this->_evs[i].data.fd;
            if (fd != this->_tsd->_listenfd) {
                this->processEvent(this->_evs[i]);
            } else {
                int conn = ::accept4(fd, NULL, NULL, SOCK_NONBLOCK);
                if (conn < 0) { fatalError("syscall accept4 error"); }
                INFO << "new connection " << conn << " accepted" << END;
                epollRemove(this->_epfd, fd);
                this->_tsd->_listenfd_lock.unlock();
                INFO << "thread release lock" << END;
                this->createBuffer(conn);
                epollRegister(this->_epfd, conn, EPOLLIN);
                this->onConnect(conn);
            }
        }
    } catch (const exception &e) { INFO << e.what() << END; }
    }
}


}

#endif
