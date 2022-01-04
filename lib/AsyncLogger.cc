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
#include <set>

#include "kingpin/Exception.h"
#include "kingpin/Buffer.h"
#include "kingpin/AsyncLogger.h"
#include "kingpin/Utils.h"

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

void AsyncLogger::_write_time() {
    string str = timestamp();
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

void AsyncLogger::_init_buffer(pid_t tid) {
    WRLockGuard lock(_map_lock);
    _t_buffers[tid] = make_tuple<>
        (shared_ptr<Buffer>(new Buffer), shared_ptr<RecursiveLock>(new RecursiveLock), 0, scTime());
}

// All the overloads except END will call "operator<<(const char *str)"
AsyncLogger &AsyncLogger::operator<<(const char *str) {
    pid_t tid = gettid();
    {
        RDLockGuard lock(_map_lock);
        if (0 == _t_buffers.count(tid)) {
            lock.unlock();
            _init_buffer(tid);
            lock.lock();
        }
        get<1>(_t_buffers[tid])->lock();
        ++get<2>(_t_buffers[tid]);
        if (1 == get<2>(_t_buffers[tid])) { _write_debug(); }
        get<0>(_t_buffers[tid])->appendToBuffer(str);
    }
    return *this;
}

// Release lock and notify cv
AsyncLogger &AsyncLogger::operator<<(const AsyncLogger &) {
    pid_t tid = gettid();
    {
        RDLockGuard lock(_map_lock);
        if (0 == _t_buffers.count(tid)) {
            lock.unlock();
            _init_buffer(tid);
            lock.lock();
        }
        get<1>(_t_buffers[tid])->lock();
        int recur_num = get<2>(_t_buffers[tid]) + 1;
        get<2>(_t_buffers[tid]) = 0;
        get<0>(_t_buffers[tid])->appendToBuffer("\n");
        get<3>(_t_buffers[tid]) = scTime();
        for (int i = 0; i < recur_num; ++i) { get<1>(_t_buffers[tid])->unlock(); }
    }
    _thr_cv.notify_one();
    return *this;
}

void AsyncLogger::_run() {
    while (true) {
        bool shall_delay = true;
        set<pid_t> remove_list;
        {
            RDLockGuard lock(_map_lock);
            for (auto iter = _t_buffers.cbegin(); iter != _t_buffers.cend(); ++iter) {
                unique_lock<RecursiveLock> lock(*((get<1>(iter->second)).get()));
                Buffer *buffer = get<0>(iter->second).get();
                if (buffer->_offset) {
                    buffer->writeNioFromBufferTillEnd(_level);
                    shall_delay = false;
                } else {
                    if (get<3>(iter->second) + _expired < scTime())
                        { remove_list.insert(iter->first); }
                }
            }
        }
        if (remove_list.size()) {
            WRLockGuard lock(_map_lock);
            for (auto iter = remove_list.cbegin(); iter != remove_list.cend(); ++iter) {
                if (0 == get<0>(_t_buffers[*iter])->_offset)
                    { _t_buffers.erase(*iter); }
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
