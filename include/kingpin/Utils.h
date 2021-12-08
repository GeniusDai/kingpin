#ifndef __UTILS_H_de3094e9a992_
#define __UTILS_H_de3094e9a992_

#include <cstddef>

namespace kingpin {

void fatalError(const char *str);

void nonFatalError(const char *str);

void fdClosedError(const char *str);

int connectAddr(struct sockaddr *addr_ptr, size_t size, int timeout);

int connectHost(const char *host, int port, int timeout);

int connectIp(const char *ip, int port, int timeout);

int initListen(int port, int listen_num);

}

#endif
