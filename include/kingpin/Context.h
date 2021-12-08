#ifndef __CONTEXT_H_f2b87e623d0d_
#define __CONTEXT_H_f2b87e623d0d_

namespace kingpin {

class Context {
public:
    Context(int timeout);
    void done();
    bool wait();
private:
    int _pfd[2];
    int _timeout;
};

class ThrWithCtx {
public:
    typedef void*(*ThrFuncPtr)(void *);
    struct ArgWithCtx {
        ThrWithCtx::ThrFuncPtr _f;
        void *_arg;
        Context *_ctx;
        ArgWithCtx(ThrFuncPtr f, void *arg, Context *ctx) : _f(f), _arg(arg), _ctx(ctx) {}
    };

    ThrWithCtx(ThrFuncPtr f, void *arg, Context *ctx);
    bool run();
    static void *_thread_wrapper(void *arg);
private:
    ThrFuncPtr _f;
    void *_arg;
    Context *_ctx;
};

}

#endif
