#ifndef __EXCEPTION_H_1_
#define __EXCEPTION_H_1_

#include <exception>
#include <string>

using namespace std;

class FatalException : public exception {
    char *_what_arg;
public:
    FatalException(const char *what_arg) {
        this->_what_arg = const_cast<char *>(what_arg);
    }

    FatalException(const string &what_arg) {
        this->_what_arg = const_cast<char *>(what_arg.c_str());
    }

    const char *what() const noexcept {
        return  _what_arg;
    }
};

class NonFatalException : public exception {
    char *_what_arg;
public:
    NonFatalException(const char *what_arg) {
        this->_what_arg = const_cast<char *>(what_arg);
    }

    NonFatalException(const string &what_arg) {
        this->_what_arg = const_cast<char *>(what_arg.c_str());
    }

    const char *what() const noexcept {
        return _what_arg;
    }
};

#endif