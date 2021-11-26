#ifndef __IOHANDLER_SERVER_H
#define __IOHANDLER_SERVER_H

#include <chrono>

#include "kingpin/IOHandler.h"
#include "kingpin/Utils.h"

using namespace std;

template <typename _ThreadSharedData>
class IOHandlerForServer : public IOHandler<_ThreadSharedData> {
public:
    IOHandlerForServer(const IOHandlerForServer &) = delete;
    IOHandlerForServer &operator=(const IOHandlerForServer &) = delete;

    IOHandlerForServer(_ThreadSharedData *tsd_ptr) : IOHandler<_ThreadSharedData>(tsd_ptr) {}

    virtual void onConnect(int conn) = 0;

    void _run() {
        INFO << "thread start" << END;
        while (true) {
            int timeout = 5;

            if (this->_tsd_ptr->_listenfd_lock.try_lock()) {
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
                    if (conn < 0) {
                        fatalError("syscall accept4 error");
                    }
                    INFO << "new connection " << conn << " accepted" << END;
                    this->RemoveFd(fd);
                    this->_tsd_ptr->_listenfd_lock.unlock();
                    INFO << "thread release lock" << END;
                    this->onConnect(conn);
                    this_thread::sleep_for(chrono::milliseconds(1));
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
        this->_t = make_shared<thread>(&IOHandlerForServer::_run, this);
    }

    virtual ~IOHandlerForServer() {}
};

#endif
