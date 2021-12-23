#include "kingpin/IOHandler.h"
#include "kingpin/TPSharedData.h"
#include "kingpin/EpollTP.h"
#include <cstring>

using namespace std;
using namespace kingpin;

template<typename _Data>
class SimpleHandler : public IOHandlerForServer<_Data> {
public:
    SimpleHandler(_Data *d) : IOHandlerForServer<_Data>(d) {}

    void onConnect(int conn) { this->writeToBuffer(conn, "Hello, kingpin!"); }

    void onWriteComplete(int conn) { ::close(conn); }
};

class SharedData : public ServerTPSharedData {};

int main() {
    SharedData data;
    data._port = 8888;
    EpollTPServer<SimpleHandler, SharedData> server(8, &data);
    server.run();
    return 0;
}