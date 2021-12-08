#ifndef __EPOLLTP_H_f2b87e623d0d_
#define __EPOLLTP_H_f2b87e623d0d_

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
#include "kingpin/Utils.h"

using namespace std;

static const int LISTEN_NUM = 1024;

template <
    template<typename _TPSharedData> class _IOHandler,
    typename _TPSharedData
>
class EpollTP final {
    vector<shared_ptr<_IOHandler<_TPSharedData> > > _handlers;
    int _thr_num;
    _TPSharedData *_tsd_ptr;
public:
    EpollTP(int thr_num, _TPSharedData *tsd_ptr) : _thr_num(thr_num), _tsd_ptr(tsd_ptr) {
        for (int i = 0; i < thr_num; ++i)
            { _handlers.emplace_back(make_shared<_IOHandler<_TPSharedData> >(_tsd_ptr)); }
    }

    void run() {
        for (auto h : _handlers) { h->run(); }
        for (auto h : _handlers) { h->join(); }
    }
};

template <
    template<typename _TPSharedData> class _IOHandler,
    typename _TPSharedData
>
class EpollTPClient final {
public:
    shared_ptr<EpollTP<_IOHandler, _TPSharedData> > _tp;

    EpollTPClient(int thr_num, _TPSharedData *tsd_ptr)
        { _tp = make_shared<EpollTP<_IOHandler, _TPSharedData> >(thr_num, tsd_ptr); }

    void run() { _tp->run(); }
};

template<
    template<typename _TPSharedData> class _IOHandler,
    typename _TPSharedData
>
class EpollTPServer final {
public:
    shared_ptr<EpollTP<_IOHandler, _TPSharedData> > _tp;

    EpollTPServer(int thr_num, int port, _TPSharedData *tsd_ptr) {
        tsd_ptr->_listenfd = initListen(port, LISTEN_NUM);
        _tp = make_shared<EpollTP<_IOHandler, _TPSharedData> >(thr_num, tsd_ptr);
    }

    void run() { _tp->run(); }
};

#endif
