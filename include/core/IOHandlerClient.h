#ifndef __IOHANDLER_CLIENT_H
#define __IOHANDLER_CLIENT_H

#include "IOHandler.h"

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
                if (this->_evs[i].events & EPOLLIN) {
                    cout << "conn " << this->_evs[i].data.fd << " readable" << endl;
                    this->onReadable(this->_evs[i].data.fd);
                }
                if (this->_evs[i].events & EPOLLRDHUP) {
                    cout << "conn " << this->_evs[i].data.fd << " passively closed" << endl;
                    this->onPassivelyClose(this->_evs[i].data.fd);
                }
                if (this->_evs[i].events & EPOLLOUT) {
                    cout << "conn " << this->_evs[i].data.fd << " writable" << endl;
                    this->onWritable(this->_evs[i].data.fd);
                }
            }
        }
    }

    void run() {
        this->_t = make_shared<thread>(IOHandlerClient::_run, this);
    }
};

#endif