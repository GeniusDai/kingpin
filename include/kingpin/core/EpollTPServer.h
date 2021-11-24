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

#include "kingpin/core/EpollTP.h"
#include "kingpin/core/Logger.h"

using namespace std;

const int LISTEN_NUM = 10;

template<
    template<typename _ThreadShareData> class _IOHandler,
    typename _ThreadShareData
>
class EpollTPServer final {
public:
    shared_ptr<EpollTP<_IOHandler, _ThreadShareData> > _tp;

    EpollTPServer(int thrNum, int port, _ThreadShareData *tsd_ptr) {
        tsd_ptr->_listenfd = _listen(port);
        _tp = make_shared<EpollTP<_IOHandler, _ThreadShareData> >(thrNum, tsd_ptr);
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
