#include <sys/epoll.h>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <assert.h>
#include <memory>
#include <tuple>
#include <unordered_map>
#include <unordered_set>

#include "kingpin/EpollTP.h"
#include "kingpin/IOHandler.h"
#include "kingpin/ThreadSharedData.h"
#include "kingpin/Exception.h"
#include "kingpin/Logger.h"

using namespace std;

class ChessGameShareData : public ThreadSharedDataServer {
public:
    mutex _m;
    unordered_map<int, int> match;
    unordered_set<int> single;
    unordered_map<int, pair<unique_ptr<char>, int> > message;
};

template <typename ChessGameShareData>
class ChessGameIOHandler : public IOHandlerForServer<ChessGameShareData> {
    static const int _msgBufferSize = 100;
    static constexpr const char *initMsg = "0 0 0 0\n";
public:
    ChessGameIOHandler(ChessGameShareData *tsd_ptr) :
        IOHandlerForServer<ChessGameShareData>(tsd_ptr) {}

    void onConnect(int conn) {
        unique_lock<mutex> lg(this->_tsd_ptr->_m);
        this->_tsd_ptr->message[conn] = make_pair(
            unique_ptr<char>(new char[_msgBufferSize]), 0);
        if (this->_tsd_ptr->single.empty()) {
            this->_tsd_ptr->single.insert(conn);
            INFO << "single client " << conn << END;
        } else {
            auto iter = this->_tsd_ptr->single.begin();
            int oppo = *iter;
            this->_tsd_ptr->single.erase(iter);
            this->_tsd_ptr->match[oppo] = conn;
            this->_tsd_ptr->match[conn] = oppo;
            ::write(oppo, initMsg, strlen(initMsg));
            INFO << "match client " << conn << " & " << oppo << END;
        }
        this->RegisterFd(conn, EPOLLIN | EPOLLRDHUP);
    }

    void onPassivelyClosed(int conn) {
        INFO << "client closed the socket, will close " << conn << END;
        this->RemoveFd(conn);
        ::close(conn);
        this->_tsd_ptr->message.erase(conn);
        if (this->_tsd_ptr->match.find(conn) != this->_tsd_ptr->match.cend()) {
            INFO << "find oppo, will close socket " << this->_tsd_ptr->match[conn] << END;
            this->RemoveFd(this->_tsd_ptr->match[conn]);
            ::close(this->_tsd_ptr->match[conn]);
            this->_tsd_ptr->message.erase(this->_tsd_ptr->match[conn]);
            this->_tsd_ptr->match.erase(this->_tsd_ptr->match[conn]);
        } else {
            this->_tsd_ptr->single.erase(conn);
        }
        this->_tsd_ptr->match.erase(conn);
    }

    void onReadable(int conn, uint32_t events) {
        unique_lock<mutex> lg(this->_tsd_ptr->_m);
        char buf[100];
        ::memset(buf, 0, 100);
        int len = read(conn, buf, 100);
        INFO << "read from " << conn << " " << buf << END;
        if (len != 0) {
            appendOrSendMessage(conn, buf, len);
        }

        if (events & EPOLLRDHUP) {
            onPassivelyClosed(conn);
        }
    }

    void appendOrSendMessage(int conn, char *buf, int len) {
        int curr = this->_tsd_ptr->message[conn].second;
        if (len + curr >= _msgBufferSize) {
            throw FatalException("buffer overflow error");
        }
        for (int i = 0; i < len; ++i) {
            this->_tsd_ptr->message[conn].first.get()[curr+i] = buf[i];
        }
        this->_tsd_ptr->message[conn].second = curr + len;
        if (this->_tsd_ptr->message[conn].first.get()[curr+len-1] == '\n') {
            INFO << "receive message from " << conn << END;
            ::write(this->_tsd_ptr->match[conn],
                this->_tsd_ptr->message[conn].first.get(),
                this->_tsd_ptr->message[conn].second);
            this->_tsd_ptr->message[conn].second = 0;
        } else {
            INFO << "receive partial message from " << conn << END;
        }
    }
};

int main(int argc, char **argv) {
    ChessGameShareData data;
    EpollTPServer<ChessGameIOHandler, ChessGameShareData> server(8, 8889, &data);
    server.run();
    return 0;
}