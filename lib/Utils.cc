#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string>
#include <cerrno>
#include <cstring>
#include <vector>
#include <cassert>
#include <fcntl.h>
#include <tuple>
#include <sys/epoll.h>
#include <sstream>
#include <signal.h>
#include "kingpin/Exception.h"
#include "kingpin/Utils.h"
#include "kingpin/AsyncLogger.h"

namespace kingpin {

void fatalError(const char *str) {
    INFO << str << ": " << ::strerror(errno) << END;
    throw FatalException(str);
}

void nonFatalError(const char *str) {
    INFO << str << ": " << ::strerror(errno) << END;
    throw NonFatalException(str);
}

void fdClosedError(const char *str) {
    INFO << str << ": " << ::strerror(errno) << END;
    throw FdClosedException();
}

void timeoutError(const char *str) {
    INFO << str << ": " << ::strerror(errno) << END;
    throw TimeoutException();
}

void dnsError(const char *str) {
    INFO << str << ": " << ::strerror(errno) << END;
    throw DNSException();
}

void setTcpSockaddr(struct sockaddr_in *addr_ptr, const char *ip, int port) {
    ::memset(addr_ptr, 0, sizeof(struct sockaddr));
    addr_ptr->sin_family = AF_INET;
    addr_ptr->sin_port = htons(port);
    int ret = ::inet_pton(AF_INET, ip, (void *)&(addr_ptr->sin_addr.s_addr));
    if (ret == -1 || ret == 0) { fatalError("Unknown error"); }
}

void getTcpHostAddr(struct sockaddr_in *addr_ptr, const char *host, int port) {
    struct addrinfo *p_info;
    struct addrinfo info;
    ::memset(&info, 0, sizeof(info));
    info.ai_family = AF_INET;
    info.ai_socktype = SOCK_STREAM;
    info.ai_flags = AI_NUMERICSERV;
    if (::getaddrinfo(host, to_string(port).c_str(), &info, &p_info) < 0) {
        dnsError("syscall getaddrinfo error");
    }
    ::memcpy(addr_ptr, p_info[0].ai_addr, sizeof(struct sockaddr_in));
    ::freeaddrinfo(p_info);
}

void getHostIp(const char *host, char *ip, size_t ip_len) {
    struct addrinfo *p_info;
    if (::getaddrinfo(host, NULL, NULL, &p_info) < 0) {
        dnsError("syscall getaddrinfo error");
    }
    struct sockaddr_in addr;
    ::memcpy(&addr, p_info[0].ai_addr, sizeof(struct sockaddr_in));
    ::freeaddrinfo(p_info);
    if (::getnameinfo((struct sockaddr *)&addr, sizeof(struct sockaddr_in), ip, ip_len,
            NULL, 0, NI_NUMERICHOST) < 0) {
        dnsError("syscall getnameinfo error");
    }
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
    int sock;
    if ((sock = ::socket(AF_INET, SOCK_STREAM, 0))== -1) {
        fatalError("syscall socket error");
    }
    if (0 == timeout) { setNonBlock(sock); }
    else if (timeout > 0) {
        struct timeval tos{timeout, 0};
        if (::setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tos, sizeof(tos)) == -1) {
            fatalError("syscall setsockopt error");
        }
    }
    if (::connect(sock, (sockaddr *)addr_ptr, sizeof(sockaddr)) < 0) {
        const char *err_msg = "syscall connect error";
        if (errno != EINPROGRESS) { nonFatalError(err_msg); }
        else if (0 != timeout) { timeoutError(err_msg); }
    }
    return sock;
}

int connectHost(const char *host, int port, int timeout) {
    struct sockaddr_in addr;
    getTcpHostAddr(&addr, host, port);
    int sock = connectAddr(&addr, timeout);
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

void epollRegister(int epfd, int fd, uint32_t events) {
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = events;
    if (::epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        stringstream ss;
        ss << "register fd " << fd << " error";
        fatalError(ss.str().c_str());
    }
}

void epollRemove(int epfd, int fd) {
    if (::epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL) == -1) {
        stringstream ss;
        ss << "remove fd " << fd << " error";
        fatalError(ss.str().c_str());
    }
}

void split(string s, string sep, vector<string>& subs) {
    if (s.size() == 0 || s.size() < sep.size()) return;
    uint start = 0;
    uint end = s.find(sep);
    while (end + sep.size() <= s.size()) {
        subs.emplace_back(s.substr(start, end));
        start = end + sep.size();
        end = s.find(sep, start);
    }
    if (start != s.size()) {
        subs.emplace_back(s.substr(start, s.size()));
    }
}

void ignoreSignal(int sig) {
    struct sigaction sigact;
    ::memset(&sigact, 0, sizeof(struct sigaction));
    sigact.sa_handler = SIG_IGN;
    sigact.sa_flags = SA_RESTART;
    if (::sigaction(sig, &sigact, NULL) == -1) {
        fatalError("ignore signal error");
    }
}

}
