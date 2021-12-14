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

template <typename _Data>
class ConcurrencyTestIOHandler : public IOHandlerForClient<_Data> {
public:
    ConcurrencyTestIOHandler(_Data *tsd) : IOHandlerForClient<_Data>(tsd) {}

    void onMessage(int conn) {
        Buffer *rb = this->_rbh[conn].get();
        INFO << "Receive message " << rb->_buffer << " from " << conn << END;
        // sleep(1);
        rb->writeNioFromBufferTillEnd(conn);
    }
};

int main() {
    ClientTPSharedData data;
    for (int i = 0; i < 800; ++i) { data.raw_add(Config::_ip, Config::_port, ""); }
    EpollTPClient<ConcurrencyTestIOHandler, ClientTPSharedData> concurrency(16, &data);
    concurrency.run();
    return 0;
}