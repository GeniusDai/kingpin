#ifndef __EXCEPTION_H_83b86438e787_
#define __EXCEPTION_H_83b86438e787_

#include <exception>
#include <string>

using namespace std;

namespace kingpin {

class FatalException : public exception {
    const char *_what_arg;
public:
    explicit FatalException(const char *what_arg) : _what_arg(what_arg){}
    explicit FatalException(const string &what_arg) : FatalException(what_arg.c_str()) {}
    virtual ~FatalException() {}

    virtual const char *what() const noexcept { return  _what_arg; }
};

class NonFatalException : public exception {
    const char *_what_arg;
public:
    NonFatalException(const char *what_arg) : _what_arg(what_arg) {}
    NonFatalException(const string &what_arg) : NonFatalException(what_arg.c_str()) {}
    virtual ~NonFatalException() {}

    virtual const char *what() const noexcept { return _what_arg; }
};

class EOFException final : public NonFatalException {
public:
    EOFException() : NonFatalException("EOF encountered") {}
};

class FdClosedException final : public NonFatalException {
public:
    FdClosedException() : NonFatalException("fd closed") {}
};

class TimeoutException final : public NonFatalException {
public:
    TimeoutException() : NonFatalException("operation timeout") {}
};

}

#endif