#ifndef __EPOLL_TP_SERVER_H
#define __EPOLL_TP_SERVER_H

#include <memory>

using namespace std;

#include "kingpin/EpollTP.h"

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

#endif
