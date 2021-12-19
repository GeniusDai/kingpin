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
#include <ctime>
#include <mutex>
#include <set>
#include "kingpin/EpollTP.h"
#include "kingpin/IOHandler.h"
#include "kingpin/TPSharedData.h"
#include "kingpin/Exception.h"
#include "kingpin/AsyncLogger.h"
#include "kingpin/Buffer.h"
#include "Config.h"

using namespace std;
using namespace kingpin;

class ChessGameShareData : public ServerTPSharedData {
public:
    mutex _m;
    unordered_map<int, int> _match;
    unordered_set<int> _single;
    map<int, string> _message;
};

template <typename ChessGameShareData>
class ChessGameIOHandler : public IOHandlerForServer<ChessGameShareData> {
public:
    ChessGameIOHandler(ChessGameShareData *tsd) :
        IOHandlerForServer<ChessGameShareData>(tsd) {}

    void onConnect(int conn) {
        unique_lock<mutex>(this->_tsd->_m);
        if (this->_tsd->_single.empty()) {
            this->_tsd->_single.insert(conn);
            INFO << "single client " << conn << " arrived"<< END;
        } else {
            auto iter = this->_tsd->_single.begin();
            this->_tsd->_match[*iter] = conn;
            this->_tsd->_match[conn] = *iter;
            INFO << "match client " << conn << " & " << *iter << END;
            ::write(conn, Config::_init_msg_black, strlen(Config::_init_msg_black));
            ::write(*iter, Config::_init_msg_red, strlen(Config::_init_msg_red));
            this->_tsd->_single.erase(*iter);
        }
    }

    void onMessage(int conn) {
        unique_lock<mutex>(this->_tsd->_m);
        if (this->_tsd->_match.find(conn) == this->_tsd->_match.end()) { return; }
        int oppo = this->_tsd->_match[conn];
        Buffer *rb = this->_rbh[conn].get();
        if (!rb->endsWith("\n")) { return; }
        INFO << "receive full message " << rb->_buffer << " from " << conn << END;
        this->_tsd->_message[oppo] = rb->_buffer;
        rb->clear();
        assert(rb->writeComplete());
    }

    void onEpollLoop() {
        unique_lock<mutex>(this->_tsd->_m);
        map<int, string> &message = this->_tsd->_message;
        set<int> dels;
        for (auto iter = message.begin(); iter != message.end(); ++iter) {
            if (this->_wbh.find(iter->first) != this->_wbh.cend()) {
                INFO << "find message for " << iter->first << END;
                this->_wbh[iter->first]->appendToBuffer(message[iter->first].c_str());
                this->writeToBuffer(iter->first);
                // erase will cause iter loses efficacy ?
                dels.insert(iter->first);
            }
        }
        for (auto fd : dels) { message.erase(fd); }
    }

    void onWriteComplete(int conn) {
        INFO << "write to " << conn << " complete" << END;
        assert(this->_wbh[conn]->writeComplete());
    }

    void onPassivelyClosed(int conn) {
        unique_lock<mutex>(this->_tsd->_m);
        int oppo = -1;
        INFO << "client " << conn << " closed the socket" << END;
        ::close(conn);
        if (this->_tsd->_match.find(conn) != this->_tsd->_match.cend()) {
            oppo = this->_tsd->_match[conn];
            this->_tsd->_match.erase(conn);
            ::write(oppo, Config::_end_msg, strlen(Config::_end_msg));
        } else {
            this->_tsd->_single.erase(conn);
        }
    }
};

int main() {
    ChessGameShareData data;
    data._port = Config::_port;
    EpollTPServer<ChessGameIOHandler, ChessGameShareData> server(8, &data);
    server.run();
    return 0;
}