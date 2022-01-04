#ifndef __LOGGER_H_de3094e9a992_
#define __LOGGER_H_de3094e9a992_

#include <unistd.h>
#include <memory>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <ctime>

#include "kingpin/Mutex.h"
#include "kingpin/Utils.h"
#include "kingpin/Buffer.h"

using namespace std;

namespace kingpin {

class AsyncLogger final {
    RWLock _map_lock;
    condition_variable_any _thr_cv;
    unique_ptr<thread> _thr_ptr;
    bool _stop = false;
public:
    int _level;
    unordered_map<pid_t, tuple<shared_ptr<Buffer>, shared_ptr<RecursiveLock>,
            int, time_t> > _t_buffers;
    time_t _expired = 120;

    explicit AsyncLogger(int level);
    ~AsyncLogger();

    AsyncLogger &operator<<(const long num);
    AsyncLogger &operator<<(const char *str);
    AsyncLogger &operator<<(const string &s);
    AsyncLogger &operator<<(const exception &e);
    AsyncLogger &operator<<(const AsyncLogger &);

    void _write_head();
    void _write_time();
    void _write_tid();
    void _write_debug();
    void _init_buffer(pid_t tid);
    bool _no_log();
    void _run();
};

extern AsyncLogger INFO;
extern AsyncLogger ERROR;
extern const AsyncLogger END;

}

#endif
