#ifndef __EPOLLTP_H_f2b87e623d0d_
#define __EPOLLTP_H_f2b87e623d0d_

#include <mutex>
#include <memory>
#include <vector>
#include <thread>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <signal.h>
#include "kingpin/AsyncLogger.h"
#include "kingpin/IOHandler.h"
#include "kingpin/Utils.h"

using namespace std;

namespace kingpin {

template <template<typename _TPSharedData> class _IOHandler,
    typename _TPSharedData>
class EpollTP final {
    using Handler = _IOHandler<_TPSharedData>;
    vector<shared_ptr<Handler> > _handlers;
    int _thr_num;
    _TPSharedData *_tsd;
public:
    EpollTP(int thr_num, _TPSharedData *tsd) : _thr_num(thr_num), _tsd(tsd) {
        for (int i = 0; i < thr_num; ++i)
            { _handlers.emplace_back(new Handler(_tsd)); }
    }

    void run() {
        ignoreSignal(SIGPIPE);
        for (auto h : _handlers) { h->run(); }
        for (auto h : _handlers) { h->join(); }
    }
};

template <template<typename _TPSharedData> class _IOHandler,
    typename _TPSharedData>
class EpollTPClient final {
    using TP = EpollTP<_IOHandler, _TPSharedData>;
public:
    unique_ptr<TP> _tp;

    EpollTPClient(int thr_num, _TPSharedData *tsd)
        { _tp = unique_ptr<TP>(new TP(thr_num, tsd)); }

    void run() { _tp->run(); }
};

template<template<typename _TPSharedData> class _IOHandler,
    typename _TPSharedData>
class EpollTPServer final {
    using TP = EpollTP<_IOHandler, _TPSharedData>;
public:
    unique_ptr<TP> _tp;

    EpollTPServer(int thr_num, _TPSharedData *tsd) {
        tsd->_listenfd = initListen(tsd->_port, tsd->_listen_num);
        _tp = unique_ptr<TP>(new TP(thr_num, tsd));
    }

    void run() { _tp->run(); }
};

}

#endif
