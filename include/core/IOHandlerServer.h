#ifndef __IOHANDLER_SERVER_H
#define __IOHANDLER_SERVER_H

#include "IOHandler.h"

template <typename _ThreadShareData>
class IOHandlerServer : public IOHandler<_ThreadShareData> {
public:
    IOHandlerServer(const IOHandlerServer &) = delete;
    IOHandlerServer &operator=(const IOHandlerServer &) = delete;

    IOHandlerServer(_ThreadShareData *tsd_ptr) : IOHandler<_ThreadShareData>(tsd_ptr) {}

    virtual void onConnect(int conn) = 0;

    void _run() {
        cout << "start thread " << this_thread::get_id() << endl;
        while (true) {
            int timeout = 10;

            if (this->_tsd_ptr->_m.try_lock()) {
                timeout = -1;
                cout << "thread " << this_thread::get_id() << " get lock" << endl;
                this->RegisterFd(this->_tsd_ptr->_listenfd, EPOLLIN);
            }

            int num = epoll_wait(this->_epfd, this->_evs, MAX_SIZE, timeout);

            for (int i = 0; i < num; ++i) {
                if (this->_evs[i].data.fd == this->_tsd_ptr->_listenfd) {
                    int conn = ::accept4(this->_tsd_ptr->_listenfd, NULL, NULL, SOCK_NONBLOCK);
                    cout << "new connection " << conn << " accepted" << endl;
                    this->RemoveFd(this->_tsd_ptr->_listenfd);
                    this->_tsd_ptr->_m.unlock();
                    cout << "thread " << this_thread::get_id() << " release lock" << endl;
                    this->onConnect(conn);
                } else {
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
    }

    void run() {
        this->_t = make_shared<thread>(&IOHandlerServer::_run, this);
    }
};

#endif
