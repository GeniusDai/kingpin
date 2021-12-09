#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string>
#include <cerrno>
#include <cassert>
#include <fcntl.h>

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

void timeoutError(const char *str) {
    ::perror(str);
    throw TimeoutException();
}

void setTcpSockaddr(struct sockaddr_in *addr_ptr, const char *ip, int port) {
    ::memset(addr_ptr, 0, sizeof(struct sockaddr));
    addr_ptr->sin_family = AF_INET;
    addr_ptr->sin_port = htons(port);
    ::inet_pton(AF_INET, ip, (void *)(static_cast<long>(addr_ptr->sin_addr.s_addr)));
}

void setNonBlock(int fd) {
    const char *str = "syscall fcntl error";
    int flags = fcntl(fd, F_GETFL);
    if (flags < 0) { fatalError(str); }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        fatalError(str);
    }
}

int connectAddr(struct sockaddr_in *addr_ptr, int timeout) {
    assert(timeout > 0);
    int sock;
    if ((sock = ::socket(AF_INET, SOCK_STREAM, 0))== -1) {
        fatalError("syscall socket error");
    }
    struct timeval tos{timeout, 0};
    if (::setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tos, sizeof(tos)) == -1) {
        fatalError("syscall setsockopt error");
    }
    if (::connect(sock, (struct sockaddr *)addr_ptr, sizeof(struct sockaddr)) < 0) {
        const char *err_msg = "syscall connect error";
        if (errno == EINPROGRESS) { timeoutError(err_msg); }
        else { fatalError(err_msg); }
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
    int sock = connectAddr((struct sockaddr_in *)(addr_ptr[0].ai_addr), timeout);
    ::freeaddrinfo(addr_ptr);
    return sock;
}

int connectIp(const char *ip, int port, int timeout) {
    struct sockaddr_in addr;
    setTcpSockaddr(&addr, ip, port);
    return connectAddr(&addr, timeout);
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
    if (::bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        fatalError("syscall bind error");
    }
    ::listen(sock, listen_num);
    return sock;
}

}
