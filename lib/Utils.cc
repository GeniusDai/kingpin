#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string>

#include "kingpin/Exception.h"

namespace kingpin {

void fatalError(const char *str) {
    ::perror(str);
    throw FatalException(str);
}

void nonFatalError(const char *str) {
    ::perror(str);
    throw NonFatalException(str);
}

void fdClosedError(const char *str) {
    ::perror(str);
    throw FdClosedException();
}

int connectAddr(struct sockaddr *addr_ptr, size_t size, int timeout) {
    int sock;
    if ((sock = ::socket(AF_INET, SOCK_STREAM, 0))== -1) {
        fatalError("syscall socket error");
    }
    if (::setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) == -1) {
        fatalError("syscall setsockopt error");
    }
    if (::connect(sock, (struct sockaddr *)addr_ptr, size) < 0) {
        fatalError("syscall connect error");
    }
    return sock;
}

int connectHost(const char *host, int port, int timeout) {
    struct addrinfo *addr_ptr;
    struct addrinfo addr;
    if (::memset(&addr, 0, sizeof(addr)) < 0) {
        fatalError("syscall memset error");
    }
    addr.ai_family = AF_INET;
    addr.ai_socktype = SOCK_STREAM;
    addr.ai_flags = AI_NUMERICSERV;
    if (::getaddrinfo(host, to_string(port).c_str(), &addr, &addr_ptr) < 0) {
        fatalError("syscall getaddrinfo error");
    }
    int sock = connectAddr((sockaddr *)addr_ptr[0].ai_addr, sizeof(struct sockaddr), timeout);
    ::freeaddrinfo(addr_ptr);
    return sock;
}

int connectIp(const char *ip, int port, int timeout) {
    struct sockaddr_in addr;
    ::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = ::htons(port);
    ::inet_pton(AF_INET, ip, &addr.sin_addr.s_addr);
    return connectAddr((sockaddr *)&addr, sizeof(addr), timeout);
}

int initListen(int port, int listen_num) {
    int sock = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { fatalError("syscall socket error"); }
    int optval = 1;
    if (::setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
        fatalError("syscall setsockopt error");
    }
    struct sockaddr_in addr;
    ::bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = ::htons(port);
    addr.sin_addr.s_addr = ::htonl(INADDR_ANY);
    if (::bind(sock, (sockaddr *)&addr, sizeof(addr)) < 0) {
        fatalError("syscall bind error");
    }
    ::listen(sock, listen_num);
    return sock;
}

}
