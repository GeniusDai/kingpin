#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <cstring>
#include <ctime>

#include "kingpin/Logger.h"
#include "kingpin/Buffer.h"
#include "kingpin/Utils.h"

using namespace std;

class Client {
public:
    static const int _port;
    static const int _step;
    static const char *const _ip;

    void start() {
        int sock = initConnect(_ip, _port);
        const char *str = "/tmp";
        ::write(sock, str, strlen(str));
        sleep(10);
        str = "/bigfile.test\n";
        ::write(sock, str, strlen(str));
        int fd = ::open("/tmp/bigfile.test.copy", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        if (fd < 0) {
            fatalError("syscall open failed");
        }
        Buffer buffer;
        sleep(10);
        while(true) {
            bool exception_caught = false;
            try { buffer.readNioToBuffer(sock, _step); }
            catch(NonFatalException &e) {
                exception_caught = true;
                INFO << e.what() << END;
            }
            buffer.writeNioFromBuffer(fd);
            if (exception_caught) { break; }
        }
        ::close(fd);
    }
};

const int _port = 8890;
const int _step = 1024 * 10;
const char *const Client::_ip = "127.0.0.1";

int main() {
    Client client;
    client.start();
    return 0;
}