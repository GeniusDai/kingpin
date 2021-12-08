#ifndef __CONTEXT_H_f2b87e623d0d_
#define __CONTEXT_H_f2b87e623d0d_

#include <pthread.h>
#include <unistd.h>
#include <sys/epoll.h>

#include "kingpin/Utils.h"

class Context {
public:
    Context(int timeout) : _timeout(timeout) {
        if (::pipe(_pfd) == -1) { fatalError("syscall pipe error"); }
    }

    void done() { ::close(_pfd[1]); }

    bool wait() {
        int epfd = ::epoll_create(1);
        if (epfd < 0) { fatalError("syscall epoll_create error"); }
        struct epoll_event ev;
        ev.data.fd = _pfd[0];
        ev.events = EPOLLRDHUP;
        if (::epoll_ctl(epfd, EPOLL_CTL_ADD, _pfd[0], &ev) < 0) {
            fatalError("syscall epoll_ctl error");
        }
        int n = ::epoll_wait(epfd, &ev, 1, _timeout);
        ::close(_pfd[0]);
        ::close(epfd);
        if (n == -1) { fatalError("syscall epoll_wait error"); }
        return n == 1;
    }
private:
    int _pfd[2];
    int _timeout;
};

class ThrWithCtx {
public:
    typedef void*(*ThrFuncPtr)(void *);

    ThrWithCtx(ThrFuncPtr f, void *arg, Context *ctx) : _f(f), _arg(arg), _ctx(ctx) {}

    struct ArgWithCtx {
        ThrWithCtx::ThrFuncPtr _f;
        void *_arg;
        Context *_ctx;
        ArgWithCtx(ThrFuncPtr f, void *arg, Context *ctx) : _f(f), _arg(arg), _ctx(ctx) {}
    };

    ThrFuncPtr _f;
    void *_arg;
    Context *_ctx;

    bool run() {
        pthread_t thr;
        ArgWithCtx argCtx(_f, _arg, _ctx);
        if (::pthread_create(&thr, NULL, _thread_wrapper, (void *)(&argCtx)) < 0) {
            fatalError("syscall pthread_create error");
        }
        bool ok = _ctx->wait();
        if (!ok && ::pthread_cancel(thr) < 0) {
            fatalError("syscall pthread_create error");
        }
        if (::pthread_join(thr, NULL) < 0) { fatalError("syscall pthread_join error"); }
        return ok;
    }

    static void *_thread_wrapper(void *arg) {
        ArgWithCtx *ptr = static_cast<ArgWithCtx *>(arg);
        ptr->_f(ptr->_arg);
        ptr->_ctx->done();
        return NULL;
    }
};

#endif
