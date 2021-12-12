#ifndef __LOGGER_H_de3094e9a992_
#define __LOGGER_H_de3094e9a992_

#include <memory>
#include <condition_variable>
#include <mutex>
#include <thread>
#include "kingpin/Buffer.h"

using namespace std;

namespace kingpin {

class AsyncLogger final {
    int _level;
    Buffer _buffer;
    recursive_mutex _m;
    condition_variable_any _thr_cv;
    shared_ptr<thread> _thr_ptr;
    int _recur_level = 0;
    bool _stop = false;
    bool _flush = true;

public:
    explicit AsyncLogger(int level);
    ~AsyncLogger();

    AsyncLogger &operator<<(const long num);
    AsyncLogger &operator<<(const char *str);
    AsyncLogger &operator<<(const exception &e);
    AsyncLogger &operator<<(const AsyncLogger &);

    void _write_head();
    void _write_time();
    void _write_tid();
    void _write_debug();
    void _run();
};

extern AsyncLogger INFO;
extern AsyncLogger ERROR;
extern const AsyncLogger END;

}

#endif
