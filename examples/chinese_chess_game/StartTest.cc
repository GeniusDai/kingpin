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

    void onMessage(int conn) {
        Buffer *rb = this->_rbh[conn].get();
        INFO << "Receive message " << rb->_buffer << " from " << conn << END;
        // sleep(1);
        rb->writeNioFromBufferTillEnd(conn);
    }
};

int main() {
    ClientTPSharedData data;
    for (int i = 0; i < 100; ++i) {
        data._pool.insert({pair<string, int>(Config::_ip, Config::_port), ""});
    }
    EpollTPClient<ConcurrencyTestIOHandler, ClientTPSharedData>
        testClient(8, &data);
    testClient.run();
    return 0;
}