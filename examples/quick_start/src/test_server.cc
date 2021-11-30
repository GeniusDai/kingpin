#include "kingpin/IOHandler.h"
#include "kingpin/TPSharedData.h"
#include "kingpin/EpollTP.h"

#include <cstring>

template<typename _Data>
class SimpleHandler : public IOHandlerForServer<_Data> {
public:
    SimpleHandler(_Data *d) : IOHandlerForServer<_Data>(d) {}
    void onConnect(int conn) {
        const char *str = "Hello, kingpin!";
        ::write(conn, str, strlen(str));
        ::close(conn);
    }
};

int main() {
    ServerTPSharedData data;
    EpollTPServer<SimpleHandler, ServerTPSharedData> server(8, 8888, &data);
    server.run();
    return 0;
}