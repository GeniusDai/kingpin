#ifndef __IOHANDLER_CLIENT_H
#define __IOHANDLER_CLIENT_H

#include "kingpin/IOHandler.h"

template <typename _ThreadSharedData>
class IOHandlerForClient : public IOHandler<_ThreadSharedData> {
public:
    IOHandlerForClient(const IOHandlerForClient &) = delete;
    IOHandlerForClient &operator=(const IOHandlerForClient &) = delete;

    IOHandlerForClient(_ThreadSharedData *tsd_ptr) : IOHandler<_ThreadSharedData>(tsd_ptr) {}

    virtual void onInit() = 0;

    void _run() {
        INFO << "thread start" << END;
        while (true) {
            int timeout = 10;

            if (this->_tsd_ptr->_m.try_lock()) {
                INFO << "thread get lock" << END;
                onInit();
                this->_tsd_ptr->_m.unlock();
                INFO << "thread release lock" << END;
                timeout = -1;
            }

            int num = epoll_wait(this->_epfd, this->_evs, MAX_SIZE, timeout);

            for (int i = 0; i < num; ++i) {
                int fd = this->_evs[i].data.fd;
                uint32_t events = this->_evs[i].events;
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

    void run() {
        this->_t = make_shared<thread>(&IOHandlerForClient::_run, this);
    }

    virtual ~IOHandlerForClient() {}
};

#endif
