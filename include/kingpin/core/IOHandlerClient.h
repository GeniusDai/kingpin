#ifndef __IOHANDLER_CLIENT_H
#define __IOHANDLER_CLIENT_H

#include "kingpin/core/IOHandler.h"

template <typename _ThreadShareData>
class IOHandlerClient : public IOHandler<_ThreadShareData> {
public:
    IOHandlerClient(const IOHandlerClient &) = delete;
    IOHandlerClient &operator=(const IOHandlerClient &) = delete;

    IOHandlerClient(_ThreadShareData *tsd_ptr) : IOHandler<_ThreadShareData>(tsd_ptr) {}

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
        this->_t = make_shared<thread>(&IOHandlerClient::_run, this);
    }

    virtual ~IOHandlerClient() {}
};

#endif
