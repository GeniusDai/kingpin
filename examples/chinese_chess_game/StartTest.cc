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
class HighConHandler : public IOHandlerForClient<_Data> {
public:
    HighConHandler(_Data *tsd) : IOHandlerForClient<_Data>(tsd) {}

    void onMessage(int conn) {
        Buffer *rb = this->_rbh[conn].get();
        INFO << "Receive message " << rb->_buffer << " from " << conn << END;
        // sleep(1);
        rb->writeNioFromBufferTillEnd(conn);
    }
};

int main() {
    ClientTPSharedData data;
    data._batch = 50;
    for (int i = 0; i < Config::_concurrency_num; ++i)
        { data.raw_add(Config::_ip, Config::_port, ""); }
    EpollTPClient<HighConHandler, ClientTPSharedData> hc_client(32, &data);
    hc_client.run();
    return 0;
}