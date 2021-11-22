#ifndef __EPOLL_TP_H_
#define __EPOLL_TP_H_

#include <mutex>
#include <memory>
#include <vector>

#include "IOHandler.h"

template <
    template<typename _ThreadShareData> class _IOHandler,
    typename _ThreadShareData
>
class EpollTP final {
    vector<shared_ptr<_IOHandler<_ThreadShareData> > > _handlers;
    int _thrNum;
    _ThreadShareData *_tsd_ptr;
public:
    EpollTP(int thrNum, _ThreadShareData *tsd_ptr) : _thrNum(thrNum), _tsd_ptr(tsd_ptr) {
        for (int i = 0; i < thrNum; ++i) {
            _handlers.emplace_back(make_shared<_IOHandler<_ThreadShareData> >(_tsd_ptr));
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

#endif