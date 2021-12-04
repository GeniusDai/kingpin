#include <unordered_map>
#include <memory>
#include <thread>
#include <chrono>

#include <fcntl.h>

#include "kingpin/TPSharedData.h"
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
        int fd;
        try {
            Buffer buffer;
            buffer.readNioToBufferTill(conn, "\n", STEP);
            buffer.stripEnd('\n');
            INFO << "client requests file [" << buffer._buffer << "]" << END;
            fd = open(buffer._buffer, O_RDONLY);
            if (fd < 0) { fatalError("syscall open failed"); }
            _hash[conn] = make_pair(fd, unique_ptr<Buffer>(new Buffer()));
            this->RegisterFd(conn, EPOLLOUT | EPOLLRDHUP);
        } catch(NonFatalException &e) {
            INFO << e << END;
        }
        sleep(5);
    }

    void onWritable(int conn, uint32_t events) {
        if (events & EPOLLRDHUP) {
            INFO << "client has closed the socket" << END;
        }
        Buffer *buffer = _hash[conn].second.get();
        int fd = _hash[conn].first;
        bool exception_caught = false;
        try {
            buffer->readNioToBuffer(fd, STEP);
            INFO << "read from disk file " << STEP << " chars" << END;
        } catch (NonFatalException &e) {
            exception_caught = true;
            INFO << e.what() << END;
        }
        try {
            buffer->writeNioFromBuffer(conn);
            INFO << "write finish" << END;
        } catch(NonFatalException &e) {
            exception_caught = true;
            INFO << e.what() << END;
        }
        if (exception_caught) {
            ::close(fd);
            this->RemoveFd(conn);
            ::close(conn);
            _hash.erase(conn);
        }
    }
};

int main() {
    ServerTPSharedData data;
    EpollTPServer<BigFileTransferIOHandler, ServerTPSharedData>
        server(8, PORT, &data);
    server.run();
    return 0;
}