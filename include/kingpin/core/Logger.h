#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <mutex>
#include <iostream>
#include <cstring>
#include <thread>
#include <condition_variable>

using namespace std;

class Logger {
public:
    recursive_mutex _m;
    static const int _buffer_size = 5 * 1024 * 1024;

    int _level;

    shared_ptr<char[]> _buffer;
    int _curr = 0;

    condition_variable_any _thr_cv;
    shared_ptr<thread> _thr_ptr;
    int _recur_level = 0;

    bool _stop = false;

    Logger(int level) : _level(level) {
        _buffer = shared_ptr<char[]>(new char[_buffer_size]);
        ::memset(static_cast<void *>(_buffer.get()), 0, sizeof(char)*_buffer_size);
        _thr_ptr = make_shared<thread>(&Logger::_run, this);
    }

    Logger &operator<<(const char *str) {
        _m.lock();
        _recur_level++;
        ::sprintf(static_cast<char *>(_buffer.get()) + _curr, "%s", str);
        _curr += ::strlen(str);
        return *this;
    }

    Logger &operator<<(const Logger &logger) {
        ::sprintf(static_cast<char *>(_buffer.get()) + _curr++, "%s", "\n");

        // Pay attention while loop may cause race condition here
        for (int i = 0; i < _recur_level-1; ++i) {
            _m.unlock();
        }
        _recur_level = 0;
        _m.unlock();

        _thr_cv.notify_one();
        return *this;
    }

    void _run() {
        while (true) {
            {
                unique_lock<recursive_mutex> lg(this->_m);
                this->_thr_cv.wait(lg, [this]()->bool{ return this->_stop || this->_curr != 0; });
                ::write(this->_level, this->_buffer.get(), _curr);
                if (this->_stop) {
                    sleep(1);
                    break;
                }
                _curr = 0;
                ::memset(static_cast<void *>(_buffer.get()), 0, sizeof(char)*_buffer_size);
            }
        }
    }

    ~Logger() {
        {
            unique_lock<recursive_mutex> lg(_m);
            _stop = true;
            _thr_cv.notify_one();
        }
        _thr_ptr->join();
    }
};

Logger END(-1);

#endif