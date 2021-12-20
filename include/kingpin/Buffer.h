#ifndef __BUFFER_H_d992a38e4d2d_
#define __BUFFER_H_d992a38e4d2d_

using namespace std;

namespace kingpin {

// NOT thread safe
class Buffer final {
private:
    unique_ptr<char []> __buffer;
public:
    int _cap;   // capacity size
    int _offset = 0;    // data size total
    int _start = 0;     // data size that have been consumed
    char *_buffer;
    int _delay = 1;
    static const int _default_cap;
    static const int _default_step;

    explicit Buffer();
    explicit Buffer(int cap);
    Buffer &operator=(const Buffer &) = delete;
    Buffer(const Buffer &) = delete;
    ~Buffer();

    void resize(int cap);
    void clear();
    int readNioToBuffer(int fd, int len);
    int readNioToBufferTillBlock(int fd);
    int readNioToBufferTillBlockOrEOF(int fd);
    int readNioToBufferTillEnd(int fd, const char *end, int step = _default_step);
    int writeNioFromBuffer(int fd, int len);
    int writeNioFromBufferTillBlock(int fd);
    int writeNioFromBufferTillEnd(int fd, int step = _default_step);
    void appendToBuffer(const char *str);
    void stripEnd(char end);
    bool writeComplete();
    bool endsWith(const char *str) const;

};

}
#endif
