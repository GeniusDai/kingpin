#include <thread>
#include <chrono>
#include <cassert>
#include <cstring>
#include <cerrno>
#include <cstdio>
#include <unistd.h>
#include <memory>
#include <functional>
#include "kingpin/Exception.h"
#include "kingpin/Utils.h"
#include "kingpin/Buffer.h"

using namespace std;

namespace kingpin {

Buffer::Buffer() : Buffer(_default_cap) {}

Buffer::Buffer(int cap) : _cap(cap) {
    __buffer = unique_ptr<char[]>(new char[_cap]);
    _buffer = __buffer.get();
    ::memset(_buffer, 0, _cap);
}

Buffer::~Buffer() {}

// resize to at least cap
void Buffer::resize(int cap) {
    if (_cap >= cap) return;
    while(_cap < cap) { _cap *= 2; }
    unique_ptr<char[]> nbuffer = unique_ptr<char[]>(new char[_cap]);
    ::memset(nbuffer.get(), 0, _cap);
    for (int i = _start; i < _offset; ++i) { nbuffer.get()[i] = _buffer[i]; }
    __buffer = move(nbuffer);
    _buffer = __buffer.get();
}

void Buffer::clear() {
    ::memset(_buffer, 0, _cap);
    _start = 0;
    _offset = 0;
}

// Read "len" bytes from NIO to buffer:
// If no data for conn or read complete, return "length"
// If read until EOF(conn closed or fd end), throw "EOFException"
// If oppo collapses, throw "fdClosedException"
int Buffer::readNioToBuffer(int fd, int len) {
    resize(_offset + len);
    int total = 0;
    const char *str = "syscall read error";
    while (true) {
        int curr = ::read(fd, _buffer + _offset, len - total);
        if (curr == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) { break; }
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

// Read all data available
int Buffer::readNioToBufferTillBlock(int fd) {
    int total = 0;
    while(true) {
        int curr = readNioToBuffer(fd, _default_step);
        total += curr;
        if (curr == 0) break;
    }
    return total;
}

int Buffer::readNioToBufferTillBlockOrEOF(int fd) {
    int start = _offset;
    try { readNioToBufferTillBlock(fd); }
    catch (const EOFException &e) {}
    return _offset - start;
}

// Read until end, if no data available, will sleep
int Buffer::readNioToBufferTillEnd(int fd, const char *end, int step) {
    assert(step > 0);
    int str_len = strlen(end);
    while(true) {
        int curr = readNioToBuffer(fd, step);
        if (curr == 0) { this_thread::sleep_for(chrono::milliseconds(_delay)); continue; }
        for (int i = max(_offset-curr, str_len-1); i <= _offset; ++i) {
            bool found = true;
            int start = i - str_len + 1;
            for (int k = 0; k < str_len; ++k) {
                if (_buffer[start+k] != end[k]) { found = false; break; }
            }
            if (found) return start;
        }
    }
}

// Write "len" bytes to NIO from buffer:
// If NIO's buffer is full or write complete, return "length"
// If oppo collapses, throw "fdClosedException"
int Buffer::writeNioFromBuffer(int fd, int len) {
    assert((len <=  _offset - _start ) && (len > 0));
    int total = 0;
    const char *str = "syscall write error";
    while (0 != len) {
        int curr = ::write(fd, _buffer + _start, len);
        if (curr == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) { break; }
            else if (errno == EPIPE || errno == ECONNRESET) {
                fdClosedError(str);
            } else { fatalError(str); }
        }
        assert (curr != 0);
        _start += curr;
        len -= curr;
        total += curr;
    }
    return total;
}

// Write until NIO's buffer is full or no more to write
int Buffer::writeNioFromBufferTillBlock(int fd) {
    int total = 0;
    int step = _default_step;
    while (_start != _offset) {
        if (step > _offset - _start) { step = _offset - _start; }
        int curr = writeNioFromBuffer(fd, step);
        total += curr;
        if (curr == 0) { break; }
    }
    return total;
}

// Write all data to buffer, if NIO's buffer is full, will sleep
int Buffer::writeNioFromBufferTillEnd(int fd, int step) {
    int total = 0;
    while (_start != _offset) {
        if (step > _offset - _start) { step = _offset - _start; }
        int curr = writeNioFromBuffer(fd, step);
        total += curr;
        if (curr == 0) { this_thread::sleep_for(chrono::milliseconds(_delay)); }
    }
    clear();
    return total;
}

void Buffer::appendToBuffer(const char *str) {
    int str_len = ::strlen(str);
    resize(_offset + str_len);
    for (int i = 0; i < str_len; ++i) { _buffer[_offset+i] = str[i]; }
    _offset += str_len;
}

void Buffer::appendToBuffer(const string &str) {
    appendToBuffer(str.c_str());
}

void Buffer::stripEnd(char end) {
    for (int i = _offset-1; i >= 0; --i) {
        if (_buffer[i] == end) { _buffer[i] = '\0'; _offset--; }
        else { break; }
    }
}

bool Buffer::writeComplete() { return _start == _offset; }

bool Buffer::endsWith(const char *str) const {
    return _offset > 0 && ::strcmp(str, _buffer + _offset - ::strlen(str)) == 0;
}

const int Buffer::_default_cap = 64;
const int Buffer::_default_step = 1024;

}
