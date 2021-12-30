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
#include <vector>
#include "kingpin/EpollTP.h"
#include "kingpin/IOHandler.h"
#include "kingpin/TPSharedData.h"
#include "kingpin/Exception.h"
#include "kingpin/AsyncLogger.h"
#include "kingpin/Buffer.h"
#include "Config.h"

using namespace std;
using namespace kingpin;

class ChessData : public TPSharedDataForServer {
public:
    recursive_mutex _m;
    unordered_map<int, int> _match;
    set<int> _single;
    map<int, string> _message;
};

template <typename ChessData>
class ChessIO : public IOHandlerForServer<ChessData> {
public:
    ChessIO(ChessData *tsd) :
        IOHandlerForServer<ChessData>(tsd) {}

    void onConnect(int conn) {
        unique_lock<recursive_mutex> lg(this->_tsd->_m);
        if (this->_tsd->_single.empty()) {
            this->_tsd->_single.insert(conn);
            INFO << "single client " << conn << " arrived"<< END;
        } else {
            auto iter = this->_tsd->_single.begin();
            int oppo = *iter;
            this->_tsd->_match[oppo] = conn;
            this->_tsd->_match[conn] = *iter;
            INFO << "match client " << conn << " & " << oppo << END;
            ::write(conn, Config::_init_msg_black, strlen(Config::_init_msg_black));
            ::write(oppo, Config::_init_msg_red, strlen(Config::_init_msg_red));
            this->_tsd->_single.erase(oppo);
        }
    }

    void onMessage(int conn) {
        unique_lock<recursive_mutex> lg(this->_tsd->_m);
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
        unique_lock<recursive_mutex> lg(this->_tsd->_m);
        map<int, string> &message = this->_tsd->_message;
        for (auto iter = message.begin(); iter != message.end(); ) {
            if (this->_wbh.find(iter->first) != this->_wbh.cend()) {
                INFO << "find message for " << iter->first << END;
                this->_wbh[iter->first]->appendToBuffer(message[iter->first].c_str());
                this->writeToBuffer(iter->first);
                iter = message.erase(iter);
            } else { ++iter; }
        }
    }

    void onWriteComplete(int conn) {
        INFO << "write to " << conn << " complete" << END;
        assert(this->_wbh[conn]->writeComplete());
    }

    void onPassivelyClosed(int conn) {
        unique_lock<recursive_mutex> lg(this->_tsd->_m);
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
    ChessData data;
    data._batch = 5;
    data._port = Config::_port;
    EpollTPServer<ChessIO, ChessData> server(32, &data);
    server.run();
    return 0;
}