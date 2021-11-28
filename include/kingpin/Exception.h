#ifndef __EXCEPTION_H_83b86438e787_
#define __EXCEPTION_H_83b86438e787_

#include <exception>
#include <string>

using namespace std;

class FatalException : public exception {
    const char *_what_arg;
public:
    FatalException(const char *what_arg) : _what_arg(what_arg){}
    FatalException(const string &what_arg) : FatalException(what_arg.c_str()) {}

    const char *what() const noexcept { return  _what_arg; }
};

class NonFatalException : public exception {
    const char *_what_arg;
public:
    NonFatalException(const char *what_arg) : _what_arg(what_arg) {}
    NonFatalException(const string &what_arg) : NonFatalException(what_arg.c_str()) {}

    const char *what() const noexcept { return _what_arg; }
};

#endif