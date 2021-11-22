#ifndef __EPOLL_TP_SERVER_H
#define __EPOLL_TP_SERVER_H

#include <memory>

using namespace std;

#include "EpollTP.h"

template <
    template<typename _ThreadShareData> class _IOHandler,
    typename _ThreadShareData
>
class EpollTPClient {
public:
    int _thrNum;
    shared_ptr<EpollTP<_IOHandler, _ThreadShareData> > _tp;
    _ThreadShareData *_tsd_ptr;

    EpollTPClient(int thrNum, _ThreadShareData *tsd_ptr) : _thrNum(thrNum), _tsd_ptr(tsd_ptr) {
        _tp = make_shared<EpollTP<_IOHandler, _ThreadShareData> >(_thrNum, _tsd_ptr);
    }

    void run() {
        _tp->run();
    }
};

#endif
