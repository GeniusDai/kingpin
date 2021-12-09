#ifndef __UTILS_H_de3094e9a992_
#define __UTILS_H_de3094e9a992_

#include <cstddef>
#include <netinet/in.h>

static_assert(sizeof(sockaddr) == sizeof(sockaddr_in));

namespace kingpin {

void fatalError(const char *str);

void nonFatalError(const char *str);

void fdClosedError(const char *str);

void setTcpSockaddr(struct sockaddr_in *addr_ptr, const char *ip, int port);

int connectAddr(struct sockaddr_in *addr_ptr, int timeout);

int connectHost(const char *host, int port, int timeout);

int connectIp(const char *ip, int port, int timeout);

int initListen(int port, int listen_num);

}

#endif
