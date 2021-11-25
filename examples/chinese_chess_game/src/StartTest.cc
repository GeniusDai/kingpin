#include <mutex>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#include "kingpin/core/EpollTPClient.h"
#include "kingpin/core/IOHandlerForClient.h"
#include "kingpin/core/ThreadSharedData.h"
#include "kingpin/core/Logger.h"
#include "kingpin/utils/Utils.h"

using namespace std;

const int POOL_SIZE = 1000;

class Data : public ThreadSharedDataClient {
public:
    bool _inited = false;
    int _thr_num = 8;
    int _port = 8889;
    const char *_ip = "127.0.0.1";
};

template <typename _Data>
class ConcurrencyTestIOHandler : public IOHandlerForClient<_Data> {
public:
    ConcurrencyTestIOHandler(_Data *tsd_ptr) : IOHandlerForClient<_Data>(tsd_ptr) {}

    void _init_conn_pool() {
        INFO << "intializing connection pool" << END;
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
            this->_tsd_ptr->_connfds.insert(sock);
            INFO << "Initialized connection of fd " << sock << END;
        }
        assert(POOL_SIZE == this->_tsd_ptr->_connfds.size());
        INFO << "Finish initializing connection pool" << END;
        this_thread::sleep_for(chrono::milliseconds(1));
    }

    void onInit() {
        if (!this->_tsd_ptr->_inited) { _init_conn_pool(); this->_tsd_ptr->_inited = true; }

        int batch = this->_tsd_ptr->_connfds.size() / this->_tsd_ptr->_thr_num + 1;

        for (auto iter = this->_tsd_ptr->_connfds.begin(); iter != this->_tsd_ptr->_connfds.end() && batch--;) {
            INFO << "Get connfd " << *iter << END;
            this->RegisterFd(*iter, EPOLLIN | EPOLLRDHUP);
            iter = this->_tsd_ptr->_connfds.erase(iter);
        }
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
        ::write(conn, buffer, size);
    }
};

int main() {
    Data data;
    EpollTPClient<ConcurrencyTestIOHandler, Data> testClient(8, &data);
    testClient.run();
    return 0;
}