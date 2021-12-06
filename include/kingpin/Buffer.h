#ifndef __BUFFER_H_d992a38e4d2d_
#define __BUFFER_H_d992a38e4d2d_

#include <thread>
#include <chrono>
#include <cassert>
#include <cstring>
#include <cerrno>
#include <cstdio>

#include "unistd.h"
#include "kingpin/Utils.h"

using namespace std;

// NOT thread safe
class Buffer final {
public:
    int _cap;   // capacity size
    int _offset = 0;    // data size total
    int _start = 0;     // data size that have been consumed
    char *_buffer;

    int _delay = 1;

    static const int _default_cap;
    static const int _default_step;

    Buffer() : Buffer(_default_cap) {}
    Buffer(int cap) : _cap(cap) { _buffer = new char[_cap]; ::memset(_buffer, 0, _cap); }
    Buffer &operator=(const Buffer &) = delete;
    Buffer(const Buffer &) = delete;
    ~Buffer() { delete _buffer; }

    // resize to at least cap
    void resize(int cap) {
        if (_cap >= cap) return;
        while(_cap < cap) { _cap *= 2; }
        char *nbuffer = new char[_cap];
        ::memset(nbuffer, 0, _cap);
        for (int i = 0; i < _offset; ++i) { nbuffer[i] = _buffer[i]; }
        delete _buffer;
        _buffer = nbuffer;
    }

    void clear() {
        ::memset(_buffer, 0, _cap);
        _start = 0;
        _offset = 0;
    }

    int readNioToBuffer(int fd, int len) {
        resize(_offset + len);
        int total = 0;
        const char *str = "syscall read error";
        while (true) {
            int curr = ::read(fd, _buffer + _offset, len - total);
            if (curr == -1) {
                if (errno == EINTR) { continue; }
                else if (errno == EAGAIN || errno == EWOULDBLOCK) { break; }
                else if (errno == ECONNRESET) {
                    fdClosedError(str);
                } else { fatalError(str); }
            }
            total += curr;
            _offset += curr;
            if (curr == 0) { throw EOFException(); }
            if (total == len) { break; }
        }
        return total;
    }

    int readNioToBufferTillBlock(int fd) {
        int total = 0;
        while(true) {
            int curr = readNioToBuffer(fd, _default_step);
            total += curr;
            if (curr == 0) break;
        }
        return total;
    }

    int readNioToBufferTillEnd(int fd, const char *end, int step = _default_step) {
        assert(step > 0);
        int str_len = strlen(end);
        while(true) {
            int curr = readNioToBuffer(fd, step);
            if (curr == 0) { this_thread::sleep_for(chrono::milliseconds(_delay)); continue; }
            for (int i = _offset-curr; i < _offset; ++i) {
                bool found = true;
                for (int k = str_len-1; k >= 0; --k) {
                    if (_buffer[i-k] != end[str_len-k-1]) { found = false; break; }
                }
                if (found) return i;
            }
        }
    }

    int writeNioFromBuffer(int fd, int len) {
        assert(len <=  _offset - _start && len > 0);
        int total = 0;
        const char *str = "syscall write error";
        while (_start != _offset) {
            int curr = ::write(fd, _buffer + _start, len);
            if (curr == -1) {
                if (errno == EINTR) { continue; }
                else if (errno == EAGAIN || errno == EWOULDBLOCK) { break; }
                else if (errno == EPIPE || errno == ECONNRESET) {
                    fdClosedError(str);
                } else { fatalError(str); }
            }
            assert (curr != 0);
            _start += curr;
            total += curr;
        }
        return total;
    }

    int writeNioFromBufferTillBlock(int fd) {
        int total = 0;
        int step = min(_offset - _start, _default_step);
        while (_start != _offset) {
            int curr = writeNioFromBuffer(fd, step);
            total += curr;
            if (curr == 0) { break; }
        }
        return total;
    }

    void writeNioFromBufferTillEnd(int fd, int step = _default_step) {
        step = min(_offset - _start, step);
        while (_start != _offset) {
            int curr = writeNioFromBuffer(fd, step);
            if (curr == 0) { this_thread::sleep_for(chrono::milliseconds(_delay)); }
        }
        clear();
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
            else { break; }
        }
    }

    bool writeComplete() { return _start == _offset; }

    bool endsWith(const char *str) const {
        return _offset > 0 && ::strcmp(str, _buffer + _offset - ::strlen(str)) == 0;
    }

};

const int Buffer::_default_cap = 64;
const int Buffer::_default_step = 1024;
#endif
