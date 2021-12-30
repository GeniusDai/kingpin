#ifndef __UTILS_H_de3094e9a992_
#define __UTILS_H_de3094e9a992_

#include <cstddef>
#include <vector>
#include <string>
#include <tuple>
#include <netinet/in.h>

using namespace std;

static_assert(sizeof(sockaddr) == sizeof(sockaddr_in));

namespace kingpin {

void fatalError(const char *str);

void nonFatalError(const char *str);

void fdClosedError(const char *str);

void timeoutError(const char *str);

void dnsError(const char *str);

void setTcpSockaddr(struct sockaddr_in *addr_ptr, const char *ip, int port);

void getTcpHostAddr(struct sockaddr_in *addr_ptr, const char *host, int port);

string getHostIp(const char *host);

void setNonBlock(int fd);

int connectAddr(struct sockaddr_in *addr_ptr, int timeout);

int connectHost(const char *host, int port, int timeout);

int connectIp(const char *ip, int port, int timeout);

int initListen(int port, int listen_num, bool nio = true);

void epollRegister(int epfd, int fd, uint32_t events);

void epollRemove(int epfd, int fd);

void split(string s, string sep, vector<string>& subs);

void ignoreSignal(int sig);

}

#endif
