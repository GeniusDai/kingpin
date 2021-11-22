#ifndef __EPOLL_TP_SERVER_H
#define __EPOLL_TP_SERVER_H

#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>

#include <mutex>
#include <thread>
#include <vector>
#include <iostream>
#include <memory>

#include "EpollTP.h"

using namespace std;

template<
    template<typename _ThreadShareData> class _IOHandler,
    typename _ThreadShareData
>
class EpollTPServer {
public:
    int _thrNum;
    int _listenfd;
    int _port;
    shared_ptr<EpollTP<_IOHandler, _ThreadShareData> > _tp;
    _ThreadShareData *_tsd_ptr;

    EpollTPServer(int thrNum, int port, _ThreadShareData *tsd_ptr) : _thrNum(thrNum), _port(port), _tsd_ptr(tsd_ptr) {
        _listenfd = _listen();
        _tp = make_shared<EpollTP<_IOHandler, _ThreadShareData> >(_thrNum, _tsd_ptr);
    }

    int _listen() {
        int sock = ::socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            throw FatalException("socket error");
        }
        struct sockaddr_in addr;
        ::bzero(&addr, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(this->_port);
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        if (::bind(sock, (sockaddr *)&addr, sizeof(addr)) < 0) {
            throw FatalException("bind error");
        }
        ::listen(sock, 2);
        cout << "listening in port " << this->_port << endl;
        return sock;
    }

    void run() {
        _tp->run();
    }
};

#endif