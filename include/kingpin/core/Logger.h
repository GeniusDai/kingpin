#ifndef __LOGGER_H__
#define __LOGGER_H__

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

#include "kingpin/core/Exception.h"

using namespace std;

class Logger final {
    int _level;

    static const int _buffer_size = 5 * 1024 * 1024;
    shared_ptr<char> _buffer;
    int _curr = 0;

    recursive_mutex _m;
    condition_variable_any _thr_cv;
    shared_ptr<thread> _thr_ptr;
    int _recur_level = 0;

    bool _stop = false;

    bool _flush = true;

public:
    bool _enable_time = true;
    bool _enable_tid = true;

    Logger(int level) : _level(level) {
        if (_level != 1 && _level != 2) return;
        _buffer = shared_ptr<char>(new char[_buffer_size]);
        ::memset(static_cast<void *>(_buffer.get()), 0, sizeof(char)*_buffer_size);
        _thr_ptr = make_shared<thread>(&Logger::_run, this);
    }

    Logger &operator<<(const long num) {
        int size = 100;
        char buffer[size];
        ::memset(buffer, 0, size);
        ::snprintf(buffer, size, "%ld", num);
        return (*this << buffer);
    }

    // All the overloads will call "operator<<(const char *str)"
    Logger &operator<<(const char *str) {
        _m.lock();
        _recur_level++;

        if (this->_flush) _write_debug();
        this->_flush = false;

        ::sprintf(_buffer.get() + _curr, "%s", str);
        _curr += ::strlen(str);
        return *this;
    }

    // Flush the stdout/stderr
    Logger &operator<<(const Logger &) {

        ::sprintf(_buffer.get() + _curr, "\n");
        _curr += 1;

        // Pay attention while loop may cause race condition here
        for (int i = 0; i < _recur_level-1; ++i) {
            _m.unlock();
        }
        _recur_level = 0;
        _flush = true;
        _m.unlock();

        _thr_cv.notify_one();
        return *this;
    }

    void _write_head() {
        const char *str = "- [INFO] ";
        assert(_level == 1 || _level == 2);
        if (_level == 2) str = "- [ERROR] ";

        ::sprintf(_buffer.get() + _curr, "%s", str);
        _curr += strlen(str);
    }

    // Not thread safe !
    void _write_time() {
        time_t t = ::time(NULL);
        if (t == -1) throw FatalException("get time failed");
        const char *str = ctime(&t); // will end by '\n'

        ::sprintf(_buffer.get() + _curr, "[%s", str);
        while(_buffer.get()[_curr] != '\n') _curr++;
        ::sprintf((_buffer.get()) + _curr, "] ");
        _curr += 2;
    }

    void _write_tid() {
        std::stringstream ss;
        long tid;
        ss << std::this_thread::get_id();
        ss >> tid;

        ::sprintf(_buffer.get() + _curr, "[TID %ld] ", tid);
        while(_buffer.get()[_curr] != '\0') _curr++;
    }

    void _write_debug() {
        _write_head();
        if (_enable_time) _write_time();
        if (_enable_tid) _write_tid();
    }

    void _run() {
        while (true) {
            {
                unique_lock<recursive_mutex> lg(this->_m);
                this->_thr_cv.wait(lg, [this]()->bool{ return this->_stop || this->_curr != 0; });

                ::write(this->_level, this->_buffer.get(), _curr);
                if (this->_stop) {
                    break;
                }
                _curr = 0;
                ::memset(_buffer.get(), 0, sizeof(char)*_buffer_size);
                assert(_flush);
            }
        }
    }

    ~Logger() {
        if (_level != 1 && _level != 2) return;
        {
            unique_lock<recursive_mutex> lg(_m);
            _stop = true;
            _thr_cv.notify_one();
        }
        _thr_ptr->join();
    }
};

Logger INFO(1);
Logger ERROR(2);
const Logger END(-1);

#endif
