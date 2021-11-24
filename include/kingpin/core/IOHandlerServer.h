#ifndef __IOHANDLER_SERVER_H
#define __IOHANDLER_SERVER_H

#include "kingpin/core/IOHandler.h"

template <typename _ThreadShareData>
class IOHandlerServer : public IOHandler<_ThreadShareData> {
public:
    IOHandlerServer(const IOHandlerServer &) = delete;
    IOHandlerServer &operator=(const IOHandlerServer &) = delete;

    IOHandlerServer(_ThreadShareData *tsd_ptr) : IOHandler<_ThreadShareData>(tsd_ptr) {}

    virtual void onConnect(int conn) = 0;

    void _run() {
        INFO << "thread start" << END;
        while (true) {
            int timeout = 10;

            if (this->_tsd_ptr->_m.try_lock()) {
                timeout = -1;
                INFO << "thread get lock" << END;
                this->RegisterFd(this->_tsd_ptr->_listenfd, EPOLLIN);
            }

            int num = epoll_wait(this->_epfd, this->_evs, MAX_SIZE, timeout);

            for (int i = 0; i < num; ++i) {
                int fd = this->_evs[i].data.fd;
                uint32_t events = this->_evs[i].events;
                if (fd == this->_tsd_ptr->_listenfd) {
                    int conn = ::accept4(fd, NULL, NULL, SOCK_NONBLOCK);
                    INFO << "new connection " << conn << " accepted" << END;
                    this->RemoveFd(fd);
                    this->_tsd_ptr->_m.unlock();
                    INFO << "thread release lock" << END;
                    this->onConnect(conn);
                } else {
                    if (events & EPOLLIN) {
                        INFO << "conn " << fd << " readable" << END;
                        this->onReadable(fd, events);
                    }
                    if (events & EPOLLOUT) {
                        INFO << "conn " << fd << " writable" << END;
                        this->onWritable(fd, events);
                    }
                }
            }
        }
    }

    void run() {
        this->_t = make_shared<thread>(&IOHandlerServer::_run, this);
    }

    virtual ~IOHandlerServer() {}
};

#endif
