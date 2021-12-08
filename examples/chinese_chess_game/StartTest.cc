#include <mutex>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#include "kingpin/EpollTP.h"
#include "kingpin/IOHandler.h"
#include "kingpin/TPSharedData.h"
#include "kingpin/AsyncLogger.h"
#include "kingpin/Utils.h"
#include "kingpin/Buffer.h"

#include "Config.h"

using namespace std;
using namespace kingpin;

const int POOL_SIZE = 10;

template <typename _Data>
class ConcurrencyTestIOHandler : public IOHandlerForClient<_Data> {
public:
    ConcurrencyTestIOHandler(_Data *tsd_ptr) : IOHandlerForClient<_Data>(tsd_ptr) {}
    bool _inited = false;
    void onInit() {
        if (_inited) return;
        _inited = true;
        INFO << "intializing thread" << END;
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(Config::_port);
        inet_pton(AF_INET, Config::_ip, &addr.sin_addr.s_addr);

        for (int i = 0; i < POOL_SIZE; ++i) {
            int sock = connectAddr((struct sockaddr *)&addr, sizeof(addr), 10);
            int flags = fcntl(sock, F_GETFL);
            if (flags < 0) { fatalError("syscall fcntl error"); }
            if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0) {
                fatalError("syscall fcntl error");
            }
            this->registerFd(sock, EPOLLIN);
            this->createBuffer(sock);
            INFO << "Initialized connection of fd " << sock << END;
        }
        INFO << "Finish initializing thread" << END;
    }

    void onMessage(int conn) {
        Buffer *rb = this->_tsd_ptr->_rbh[conn].get();
        INFO << "Receive message " << rb->_buffer << " from " << conn << END;
        rb->writeNioFromBufferTillEnd(conn);
    }
};

int main() {
    ClientTPSharedData data;
    EpollTPClient<ConcurrencyTestIOHandler, ClientTPSharedData>
        testClient(8, &data);
    testClient.run();
    return 0;
}