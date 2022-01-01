#include <string>
#include <mutex>
#include <iostream>
#include <cstring>
#include <thread>
#include <condition_variable>
#include <ctime>
#include <cstdio>
#include <sstream>
#include <cassert>

#include "kingpin/Exception.h"
#include "kingpin/Buffer.h"
#include "kingpin/AsyncLogger.h"

using namespace std;

namespace kingpin {

AsyncLogger::AsyncLogger(int level) : _level(level) {
    if (_level != 1 && _level != 2) { return; }
    _thr_ptr = unique_ptr<thread>(new thread(&AsyncLogger::_run, this));
}

void AsyncLogger::_write_head() {
    assert(_level == 1 || _level == 2);
    const char *str = "- [INFO] ";
    if (_level == 2) { str = "- [ERROR] "; }
    pid_t id = gettid();
    get<0>(_t_buffers[id])->appendToBuffer(str);
}

// Not thread safe !
void AsyncLogger::_write_time() {
    time_t t = ::time(NULL);
    if (t == -1) { throw FatalException("get time failed"); }
    char *str = ctime(&t); // will end by '\n'
    str[::strlen(str)-1] = '\0';
    pid_t id = gettid();
    get<0>(_t_buffers[id])->appendToBuffer("[");
    get<0>(_t_buffers[id])->appendToBuffer(str);
    get<0>(_t_buffers[id])->appendToBuffer("] ");
}

void AsyncLogger::_write_tid() {
    std::stringstream ss; ss << this_thread::get_id();
    pid_t id = gettid();
    get<0>(_t_buffers[id])->appendToBuffer("[TID ");
    get<0>(_t_buffers[id])->appendToBuffer(ss.str().c_str());
    get<0>(_t_buffers[id])->appendToBuffer("] ");
}

void AsyncLogger::_write_debug() {
    _write_head();
    _write_time();
    _write_tid();
}

bool AsyncLogger::_no_log() {
    for (auto iter = _t_buffers.cbegin(); iter != _t_buffers.cend(); ++iter) {
        if (get<0>(iter->second).get()->_offset) { return false; }
    }
    return true;
}

AsyncLogger &AsyncLogger::operator<<(const long num) {
    stringstream ss;
    ss << num;
    return (*this << ss.str().c_str());
}

AsyncLogger &AsyncLogger::operator<<(const string &s) {
    return (*this << s.c_str());
}

AsyncLogger &AsyncLogger::operator<<(const exception &e) {
    return (*this << e.what());
}

void AsyncLogger::_check_or_init_tid_buffer() {
    pid_t id = gettid();
    {
        RDLockGuard lock(_map_lock);
        if (_t_buffers.count(id)) { return; }
    }
    WRLockGuard lock(_map_lock);
    _t_buffers[id] = make_tuple<>
        (shared_ptr<Buffer>(new Buffer), shared_ptr<RecursiveLock>(new RecursiveLock), 0);
}

// All the overloads except END will call "operator<<(const char *str)"
AsyncLogger &AsyncLogger::operator<<(const char *str) {
    _check_or_init_tid_buffer();
    {
        RDLockGuard lock(_map_lock);
        pid_t id = gettid();
        get<1>(_t_buffers[id])->lock();
        ++get<2>(_t_buffers[id]);
        if (1 == get<2>(_t_buffers[id])) { _write_debug(); }
        get<0>(_t_buffers[id])->appendToBuffer(str);
    }
    return *this;
}

// Release lock and notify cv
AsyncLogger &AsyncLogger::operator<<(const AsyncLogger &) {
    _check_or_init_tid_buffer();
    {
        RDLockGuard lock(_map_lock);
        pid_t id = gettid();
        get<1>(_t_buffers[id])->lock();
        int recur_num = get<2>(_t_buffers[id]) + 1;
        get<2>(_t_buffers[id]) = 0;
        get<0>(_t_buffers[id])->appendToBuffer("\n");
        for (int i = 0; i < recur_num; ++i) { get<1>(_t_buffers[id])->unlock(); }
    }
    _thr_cv.notify_one();
    return *this;
}

void AsyncLogger::_run() {
    while (true) {
        bool shall_delay = true;
        {
            RDLockGuard lock(_map_lock);
            for (auto iter = _t_buffers.cbegin(); iter != _t_buffers.cend(); ++iter) {
                RecursiveLock *elem_lock = (get<1>(iter->second)).get();
                elem_lock->lock();
                Buffer *buffer = get<0>(iter->second).get();
                if (buffer->_offset) {
                    buffer->writeNioFromBufferTillEnd(_level);
                    shall_delay = false;
                }
                elem_lock->unlock();
            }
        }
        if (shall_delay) {
            WRLockGuard lock(_map_lock);
            shall_delay = _no_log();
            if (shall_delay && _stop) { break; }
        }
        if (shall_delay) {
            RDLockGuard lock(_map_lock);
            _thr_cv.wait(lock);
        }
    }
}

AsyncLogger::~AsyncLogger() {
    if (_level != 1 && _level != 2) { return; }
    {
        RDLockGuard map_lock(_map_lock);
        _stop = true;
    }
    _thr_cv.notify_one();
    _thr_ptr->join();
}

AsyncLogger INFO(1);
AsyncLogger ERROR(2);
const AsyncLogger END(-1);

}
