#ifndef __BUFFER_H_d992a38e4d2d_
#define __BUFFER_H_d992a38e4d2d_

#include <thread>
#include <chrono>

#include <cassert>
#include <cstring>

#include "unistd.h"

using namespace std;

static const int DEFAULT_CAP = 64;

// NOT thread safe
class Buffer final {
public:
    int _cap;   // capacity size
    int _offset = 0;    // data size total
    int _start = 0;     // data size that have been consumed
    char *_buffer;

    int _delay = 1;

    Buffer() : Buffer(DEFAULT_CAP) {}

    Buffer(int cap) : _cap(cap) { _buffer = new char[_cap]; ::memset(_buffer, 0, _cap); }

    ~Buffer() { delete _buffer; }

    void resize(int cap) {
        if (_cap >= cap) return;
        while(_cap < cap) { _cap *= 2; }
        char *nbuffer = new char[_cap];
        ::memset(nbuffer, 0, _cap);
        for (int i = 0; i < _offset; ++i) { nbuffer[i] = _buffer[i]; }
        delete _buffer;
        _buffer = nbuffer;
    }

    int readNioToBuffer(int fd, int len) {
        resize(_offset + len);
        int curr = ::read(fd, _buffer + _offset, len);
        _offset += curr;
        return curr;
    }

    void readNioToBufferTill(int fd, char end, int step) {
        assert(step > 0);
        while(true) {
            int curr = readNioToBuffer(fd, step);
            if (end == '\0') {
                if (curr < step) { break; }
            } else {
                if (_offset != 0 && _buffer[_offset-1] == end) { break; }
            }
            this_thread::sleep_for(chrono::milliseconds(_delay));
        }
    }

    void writeNioFromBuffer(int fd) {
        while (true) {
            int curr = ::write(fd, _buffer + _start, _offset - _start);
            _start += curr;
            if (_start == _offset) { break; }
            this_thread::sleep_for(chrono::milliseconds(_delay));
        }
        ::memset(_buffer, 0, _cap);
        _start = 0;
        _offset = 0;
    }

    void appendToBuffer(const char *str) {
        int str_len = ::strlen(str);
        resize(_offset + str_len);
        for (int i = 0; i < str_len; ++i) { _buffer[_offset+i] = str[i]; }
        _offset += str_len;
    }

    void stripEnd(char end) {
        for (int i = _offset-1; i >= 0; --i) {
            if (_buffer[i] == end) { _buffer[i] = '\0'; _offset--; }
            else break;
        }
    }

};

#endif