#include <pthread.h>
#include <unistd.h>
#include <sys/epoll.h>

#include "kingpin/Context.h"
#include "kingpin/Utils.h"

namespace kingpin {

Context::Context(int timeout) : _timeout(timeout) {
    if (::pipe(_pfd) == -1) { fatalError("syscall pipe error"); }
}

void Context::done() { ::close(_pfd[1]); }

bool Context::wait() {
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

ThrWithCtx::ThrWithCtx(ThrFuncPtr f, void *arg, Context *ctx) : _f(f), _arg(arg), _ctx(ctx) {}

bool ThrWithCtx::run() {
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

void *ThrWithCtx::_thread_wrapper(void *arg) {
    ArgWithCtx *ptr = static_cast<ArgWithCtx *>(arg);
    ptr->_f(ptr->_arg);
    ptr->_ctx->done();
    return NULL;
}

}
