#ifndef __EPOLL_TP_H_
#define __EPOLL_TP_H_

#include <mutex>
#include <memory>
#include <vector>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>

#include <mutex>
#include <thread>
#include <vector>
#include <iostream>
#include <memory>

#include "kingpin/Logger.h"
#include "kingpin/IOHandler.h"

using namespace std;

const int LISTEN_NUM = 1024;

template <
    template<typename _ThreadSharedData> class _IOHandler,
    typename _ThreadSharedData
>
class EpollTP final {
    vector<shared_ptr<_IOHandler<_ThreadSharedData> > > _handlers;
    int _thrNum;
    _ThreadSharedData *_tsd_ptr;
public:
    EpollTP(int thrNum, _ThreadSharedData *tsd_ptr) : _thrNum(thrNum), _tsd_ptr(tsd_ptr) {
        for (int i = 0; i < thrNum; ++i) {
            _handlers.emplace_back(make_shared<_IOHandler<_ThreadSharedData> >(_tsd_ptr));
        }
    }

    void run() {
        for (auto h : _handlers) {
            h->run();
        }

        for (auto h : _handlers) {
            h->join();
        }
    }
};

template <
    template<typename _ThreadSharedData> class _IOHandler,
    typename _ThreadSharedData
>
class EpollTPClient final {
public:
    shared_ptr<EpollTP<_IOHandler, _ThreadSharedData> > _tp;

    EpollTPClient(int thrNum, _ThreadSharedData *tsd_ptr) {
        _tp = make_shared<EpollTP<_IOHandler, _ThreadSharedData> >(thrNum, tsd_ptr);
    }

    void run() {
        _tp->run();
    }
};

template<
    template<typename _ThreadSharedData> class _IOHandler,
    typename _ThreadSharedData
>
class EpollTPServer final {
public:
    shared_ptr<EpollTP<_IOHandler, _ThreadSharedData> > _tp;

    EpollTPServer(int thrNum, int port, _ThreadSharedData *tsd_ptr) {
        tsd_ptr->_listenfd = _listen(port);
        _tp = make_shared<EpollTP<_IOHandler, _ThreadSharedData> >(thrNum, tsd_ptr);
    }

    int _listen(int port) {
        int sock = ::socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            throw FatalException("socket error");
        }
        struct sockaddr_in addr;
        ::bzero(&addr, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        if (::bind(sock, (sockaddr *)&addr, sizeof(addr)) < 0) {
            throw FatalException("bind error");
        }
        ::listen(sock, LISTEN_NUM);
        INFO << "listening in port " << port << END;
        return sock;
    }

    void run() {
        _tp->run();
    }
};

#endif