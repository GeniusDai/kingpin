#include <unordered_map>
#include <memory>
#include <thread>
#include <chrono>

#include <fcntl.h>

#include "kingpin/ThreadSharedData.h"
#include "kingpin/EpollTP.h"
#include "kingpin/IOHandler.h"
#include "kingpin/Buffer.h"

using namespace std;

static const int PORT = 8890;
static const int STEP = 1024 * 100;

template <typename _Data>
class BigFileTransferIOHandler : public IOHandlerForServer<_Data> {
    unordered_map<int, pair<int, unique_ptr<Buffer> > > _hash;
public:
    BigFileTransferIOHandler(_Data *tsd_ptr) : IOHandlerForServer<_Data>(tsd_ptr) {}
    void onConnect(int conn) {
        Buffer buffer;
        buffer.appendToBuffer("/");
        buffer.readNioToBufferTill(conn, "\n", STEP);
        buffer.stripEnd('\n');
        assert(buffer._buffer[0] == '/');
        INFO << "client requests file [" << buffer._buffer << "]" << END;
        this_thread::sleep_for(chrono::milliseconds(1));
        int fd = open(buffer._buffer, O_RDONLY);
        if (fd < 0) { fatalError("syscall open failed"); }
        _hash[conn] = make_pair(
            fd, unique_ptr<Buffer>(new Buffer()));
        this->RegisterFd(conn, EPOLLOUT);
    }

    void onWritable(int conn, uint32_t events) {
        Buffer *buffer = _hash[conn].second.get();
        int fd = _hash[conn].first;
        int len = buffer->readNioToBuffer(fd, STEP);
        INFO << "read from disk file " << STEP << " chars" << END;
        if (len == 0) {
            ::close(conn);
            ::close(fd);
            _hash.erase(conn);
            return;
        }
        buffer->writeNioFromBuffer(conn);
        INFO << "write finish" << END;
    }
};

int main() {
    ThreadSharedDataServer data;
    EpollTPServer<BigFileTransferIOHandler, ThreadSharedDataServer>
        server(8, PORT, &data);
    server.run();
    return 0;
}