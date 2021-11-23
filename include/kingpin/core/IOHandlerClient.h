#ifndef __IOHANDLER_CLIENT_H
#define __IOHANDLER_CLIENT_H

#include "kingpin/core/IOHandler.h"

template <typename _ThreadShareData>
class IOHandlerClient {
public:
    IOHandlerClient(const IOHandlerClient &) = delete;
    IOHandlerClient &operator=(const IOHandlerClient &) = delete;

    IOHandlerClient(_ThreadShareData *tsd_ptr) : IOHandler<_ThreadShareData>(*tsd_ptr) {}

    virtual void onInit() = 0;

    void _run() {
        cout << "start thread " << this_thread::get_id() << endl;
        while (true) {
            int timeout = 10;

            int num = epoll_wait(this->_epfd, this->_evs, MAX_SIZE, 10);

            for (int i = 0; i < num; ++i) {
                int fd = this->_evs[i].data.fd;
                uint32_t events = this->_evs[i].events;
                if (events & EPOLLIN) {
                    cout << "conn " << fd << " readable" << endl;
                    this->onReadable(fd, events);
                }
                if (events & EPOLLOUT) {
                    cout << "conn " << fd << " writable" << endl;
                    this->onWritable(fd, events);
                }
            }
        }
    }

    void run() {
        this->_t = make_shared<thread>(IOHandlerClient::_run, this);
    }
};

#endif