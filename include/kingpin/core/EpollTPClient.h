#ifndef __EPOLL_TP_SERVER_H
#define __EPOLL_TP_SERVER_H

#include <memory>

using namespace std;

#include "kingpin/core/EpollTP.h"

template <
    template<typename _ThreadShareData> class _IOHandler,
    typename _ThreadShareData
>
class EpollTPClient {
public:
    shared_ptr<EpollTP<_IOHandler, _ThreadShareData> > _tp;

    EpollTPClient(int thrNum, _ThreadShareData *tsd_ptr) {
        _tp = make_shared<EpollTP<_IOHandler, _ThreadShareData> >(thrNum, tsd_ptr);
    }

    void run() {
        _tp->run();
    }
};

#endif
