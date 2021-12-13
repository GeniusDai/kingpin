#include <unordered_map>
#include <memory>
#include <thread>
#include <chrono>
#include <unistd.h>

#include <fcntl.h>

#include "kingpin/TPSharedData.h"
#include "kingpin/EpollTP.h"
#include "kingpin/IOHandler.h"
#include "kingpin/Buffer.h"
#include "kingpin/Exception.h"

using namespace std;
using namespace kingpin;

static const int PORT = 8890;

template <typename _Data>
class BigFileTransferIOHandler : public IOHandlerForServer<_Data> {
public:
    BigFileTransferIOHandler(_Data *tsd_ptr) : IOHandlerForServer<_Data>(tsd_ptr) {}
    void onMessage(int conn) {
        int fd;
        Buffer *rb, *wb;
        try {
            rb = this->_rbh[conn].get();
            wb = this->_wbh[conn].get();
            rb->stripEnd('\n');
            INFO << "client requests file [" << rb->_buffer << "]" << END;
            fd = open(rb->_buffer, O_RDONLY);
            if (fd < 0) { fatalError("syscall open failed"); }
            wb->readNioToBufferTillBlock(fd);
            INFO << "load file completed" << END;
        } catch(const EOFException &e) {
            INFO << e << END;
        }
        this->writeToBuffer(conn);
    }

    void onWriteComplete(int conn) {
        INFO << "write complete, will close socket" << END;
        ::close(conn);
    }
};

int main() {
    ServerTPSharedData data;
    EpollTPServer<BigFileTransferIOHandler, ServerTPSharedData>
        server(8, PORT, &data);
    server.run();
    return 0;
}