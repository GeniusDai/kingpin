#include <mutex>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#include "kingpin/EpollTP.h"
#include "kingpin/IOHandler.h"
#include "kingpin/ThreadSharedData.h"
#include "kingpin/Logger.h"
#include "kingpin/Utils.h"

using namespace std;

const int POOL_SIZE = 100;

class Data : public ThreadSharedDataClient {
public:
    int _port = 8889;
    const char *_ip = "127.0.0.1";
};

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
        addr.sin_port = htons(this->_tsd_ptr->_port);
        inet_pton(AF_INET, this->_tsd_ptr->_ip, &addr.sin_addr.s_addr);

        for (int i = 0; i < POOL_SIZE; ++i) {
            int sock = socket(AF_INET, SOCK_STREAM, 0);
            if (sock < 0) {
                fatalError("syscall socket error");
            }
            int flags = fcntl(sock, F_GETFL, 0);
            fcntl(sock, F_SETFD, flags | O_NONBLOCK);
            if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
                fatalError("syscall connect error");
            }
            this->RegisterFd(sock, EPOLLIN | EPOLLRDHUP);
            INFO << "Initialized connection of fd " << sock << END;
        }
        INFO << "Finish initializing thread" << END;
    }

    void onReadable(int conn, uint32_t events) {
        char buffer[100];
        memset(buffer, 0, 100);
        while (::read(conn, buffer, 100) == 0) {

        }
        int count = -1;
        int size = -1;
        int i = 0;
        for (; i < 100 && buffer[i] != '\0'; ++i) {
            if (buffer[i] == '\n') count = i;
        }
        size = i;
        if (count != -1) buffer[count] = '\0';
        INFO << "receive message " << buffer << END;
        if (count != -1) buffer[count] = '\n';
        ::write(conn, buffer, size - 3);
        ::write(conn, buffer + size - 3, 3);
    }
};

int main() {
    Data data;
    EpollTPClient<ConcurrencyTestIOHandler, Data> testClient(8, &data);
    testClient.run();
    return 0;
}