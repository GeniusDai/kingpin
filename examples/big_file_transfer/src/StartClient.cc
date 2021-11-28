#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <cstring>
#include <ctime>

#include "kingpin/Buffer.h"
#include "kingpin/Utils.h"

using namespace std;

class Client {
public:
    static const int _port = 8890;
    static const int _step = 1024 * 1000;
    static constexpr char *const _ip = "127.0.0.1";

    void start() {
        int sock = ::socket(AF_INET, SOCK_STREAM, 0);
        if (sock == -1) {
            throw "socket error";
        }
        struct sockaddr_in addr;
        ::memset(&addr, 0, sizeof(addr));
        addr.sin_port = htons(_port);
        addr.sin_family = AF_INET;
        inet_pton(AF_INET, _ip, &addr.sin_addr.s_addr);
        if (::connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            throw "connect error";
        }
        const char *str = "/tmp/bigfile\n";
        ::write(sock, str, strlen(str));
        int fd = ::open("/tmp/bigfile_copy", O_WRONLY | O_CREAT);
        if (fd < 0) {
            fatalError("syscall open failed");
        }
        Buffer buffer;
        while(true) {
            int curr = buffer.readNioToBuffer(sock, _step);
            if (curr == 0) break;
            // sleep(1);
            buffer.writeNioFromBuffer(fd);
        }
        ::close(fd);
    }
};

int main() {
    Client client;
    client.start();
    return 0;
}