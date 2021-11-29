#ifndef __LOGGER_H_de3094e9a992_
#define __LOGGER_H_de3094e9a992_

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

using namespace std;

class Logger final {
    int _level;

    Buffer _buffer;

    recursive_mutex _m;
    condition_variable_any _thr_cv;
    shared_ptr<thread> _thr_ptr;
    int _recur_level = 0;
    bool _stop = false;
    bool _flush = true;

public:
    Logger(int level) : _level(level) {
        if (_level != 1 && _level != 2) return;
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

        _buffer.appendToBuffer(str);
        return *this;
    }

    Logger &operator<<(const exception &e) {
        return (*this << e.what());
    }

    // Release lock and notify cv
    Logger &operator<<(const Logger &) {
        _buffer.appendToBuffer("\n");

        // Pay attention while loop may cause race condition here
        for (int i = 0; i < _recur_level-1; ++i) { _m.unlock(); }
        _recur_level = 0;
        _flush = true;
        _m.unlock();

        _thr_cv.notify_one();
        return *this;
    }

    void _write_head() {
        assert(_level == 1 || _level == 2);
        const char *str = "- [INFO] ";
        if (_level == 2) str = "- [ERROR] ";
        _buffer.appendToBuffer(str);
    }

    // Not thread safe !
    void _write_time() {
        time_t t = ::time(NULL);
        if (t == -1) throw FatalException("get time failed");
        char *str = ctime(&t); // will end by '\n'
        str[::strlen(str)-1] = '\0';
        _buffer.appendToBuffer("[");
        _buffer.appendToBuffer(str);
        _buffer.appendToBuffer("] ");
    }

    void _write_tid() {
        std::stringstream ss;
        ss << std::this_thread::get_id();
        _buffer.appendToBuffer("[TID ");
        _buffer.appendToBuffer(ss.str().c_str());
        _buffer.appendToBuffer("] ");
    }

    void _write_debug() {
        _write_head();
        _write_time();
        _write_tid();
    }

    void _run() {
        while (true) {
            unique_lock<recursive_mutex> lg(this->_m);
            this->_thr_cv.wait(lg, [this]()->bool
                { return this->_stop || _buffer._offset != 0; });

            _buffer.writeNioFromBuffer(this->_level);
            if (this->_stop) { break; }
            assert(_flush);
        }
    }

    ~Logger() {
        if (_level != 1 && _level != 2) return;
        {
            unique_lock<recursive_mutex> lg(_m);
            _stop = true;
        }
        _thr_cv.notify_one();
        _thr_ptr->join();
    }
};

Logger INFO(1);
Logger ERROR(2);
const Logger END(-1);

#endif
