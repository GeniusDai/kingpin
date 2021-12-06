#ifndef __UTILS_H_de3094e9a992_
#define __UTILS_H_de3094e9a992_

#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <netinet/in.h>

#include "kingpin/Exception.h"

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

int initConnect(const char *ip, int port) {
    int sock;
    if ((sock = ::socket(AF_INET, SOCK_STREAM, 0))== -1)
        { fatalError("syscall socket error"); }
    struct sockaddr_in addr;
    ::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = ::htons(port);
    ::inet_pton(AF_INET, ip, &addr.sin_addr.s_addr);
    if (::connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        { fatalError("syscall connect error"); }
    return sock;
}

int initListen(int port, int listen_num) {
    int sock = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { fatalError("syscall socket error"); }
    struct sockaddr_in addr;
    ::bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = ::htons(port);
    addr.sin_addr.s_addr = ::htonl(INADDR_ANY);
    if (::bind(sock, (sockaddr *)&addr, sizeof(addr)) < 0)
        { fatalError("syscall bind error"); }
    ::listen(sock, listen_num);
    return sock;
}

#endif
