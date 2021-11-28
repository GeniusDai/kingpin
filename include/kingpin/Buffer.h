#ifndef __BUFFER_H_d992a38e4d2d_
#define __BUFFER_H_d992a38e4d2d_

#include <thread>
#include <chrono>

#include <cassert>
#include <cstring>
#include <cerrno>

#include "unistd.h"
#include "kingpin/Utils.h"

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
    Buffer &operator=(const Buffer &) = delete;
    Buffer(const Buffer &) = delete;
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
        int total = 0;
        while (true) {
            int curr = ::read(fd, _buffer + _offset, len - total);
            if (curr == -1) {
                if (errno == EINTR) { continue; }
                else if (errno == EAGAIN || errno == EWOULDBLOCK) { break; }
                else fatalError("syscall read error");
            }
            total += curr;
            _offset += curr;
            if (curr == 0) { throw NonFatalException("EOF encountered"); }
            if (total == len) { break; }
        }

        return total;
    }

    int readNioToBufferTill(int fd, const char *end, int step) {
        assert(step > 0);
        int str_len = strlen(end);
        while(true) {
            int curr = readNioToBuffer(fd, step);
            if (strcmp(end, "\0") == 0) { return _offset; }
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

    void writeNioFromBuffer(int fd) {
        while (_start != _offset) {
            int curr = ::write(fd, _buffer + _start, _offset - _start);
            if (curr == -1) {
                if (errno == EINTR) { continue; }
                else if (errno == EPIPE || errno == ECONNRESET)
                    { nonFatalError("syscall write error: oppo closed"); }
                else if (errno == EAGAIN || errno == EWOULDBLOCK)
                    { this_thread::sleep_for(chrono::milliseconds(_delay)); }
            }
            assert (curr != 0);
            _start += curr;
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