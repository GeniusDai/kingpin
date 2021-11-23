#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <mutex>
#include <iostream>

using namespace std;

class Logger {
public:
    mutex _m;
    int _level;

    Logger(int level) : _level(level) {

    }

    Logger &operator <<(const char *str) {
        {
            unique_lock<mutex>(_m);
            cout << str << endl;
        }
        return *this;
    }
};

#endif