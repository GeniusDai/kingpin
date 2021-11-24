#include <mutex>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#include "kingpin/core/EpollTPClient.h"
#include "kingpin/core/IOHandlerClient.h"
#include "kingpin/core/ThreadShareData.h"
#include "kingpin/core/Logger.h"

using namespace std;

class Data : public ThreadShareDataClient {
public:
    bool _inited = false;
    int _thr_num = 8;
    int _port = 8889;
    const char *_ip = "127.0.0.1";
};

template <typename _Data>
class ConcurrencyTestIOHandler : public IOHandlerClient<_Data> {
public:
    ConcurrencyTestIOHandler(_Data *tsd_ptr) : IOHandlerClient<_Data>(tsd_ptr) {}

    void _init_conn_pool() {
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(this->_tsd_ptr->_port);
        inet_pton(AF_INET, this->_tsd_ptr->_ip, &addr.sin_addr.s_addr);

        for (int i = 0; i < 1000; ++i) {
            int sock = socket(AF_INET, SOCK_STREAM, 0);
            int flags = fcntl(sock, F_GETFL, 0);
            fcntl(sock, F_SETFD, flags | O_NONBLOCK);
            if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
                const char *const str = "connect error";
                perror(str);
                throw FatalException(str);
            }
            this->_tsd_ptr->_connfds.insert(sock);
        }
    }

    void onInit() {
        if (!this->_tsd_ptr->_inited) { _init_conn_pool(); this->_tsd_ptr->_inited = true; }

        unordered_set<int> &pool = this->_tsd_ptr->_connfds;
        int batch = pool.size() / this->_tsd_ptr->_thr_num + 1;

        for (auto iter = pool.begin(); iter != pool.end() && batch--; ++iter) {
            this->RegisterFd(*iter, EPOLLIN | EPOLLRDHUP);
            pool.erase(iter);
        }

    }

    void onReadable(int conn, uint32_t events) {
        char buffer[100];
        ::read(conn, buffer, 100);
        int count = -1;
        int size = -1;
        int i = 0;
        for (; i < 100 && buffer[i] != '\0'; ++i) {
            if (buffer[i] == '\n') count = i;
        }
        size = i;
        if (count != -1) buffer[count] = '\n';
        INFO << "receive message " << buffer << END;
        if (count != -1) buffer[count] = '\0';
        ::write(conn, buffer, size);
    }

    void onWritable(int conn, uint32_t events) {}
    void onPassivelyClosed(int conn) {}
};

int main() {
    Data data;
    EpollTPClient<ConcurrencyTestIOHandler, Data> testClient(8, &data);
    testClient.run();
    return 0;
}