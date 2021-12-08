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

#include "kingpin/EpollTP.h"
#include "kingpin/IOHandler.h"
#include "kingpin/TPSharedData.h"
#include "kingpin/Exception.h"
#include "kingpin/Logger.h"
#include "kingpin/Buffer.h"

#include "Config.h"

using namespace std;
using namespace kingpin;

class ChessGameShareData : public ServerTPSharedData {
public:
    mutex _m;
    unordered_map<int, int> _match;
    unordered_set<int> _single;
};

template <typename ChessGameShareData>
class ChessGameIOHandler : public IOHandlerForServer<ChessGameShareData> {
public:
    ChessGameIOHandler(ChessGameShareData *tsd_ptr) :
        IOHandlerForServer<ChessGameShareData>(tsd_ptr) {}

    void onConnect(int conn) {
        unique_lock<mutex>(this->_tsd_ptr->_m);
        if (this->_tsd_ptr->_single.empty()) {
            this->_tsd_ptr->_single.insert(conn);
            INFO << "single client " << conn << " arrived"<< END;
        } else {
            auto iter = this->_tsd_ptr->_single.begin();
            this->_tsd_ptr->_match[*iter] = conn;
            this->_tsd_ptr->_match[conn] = *iter;
            INFO << "match client " << conn << " & " << *iter << END;
            ::write(conn, Config::_init_msg_black, strlen(Config::_init_msg_black));
            ::write(*iter, Config::_init_msg_red, strlen(Config::_init_msg_red));
            this->_tsd_ptr->_single.erase(*iter);
        }
    }

    void onMessage(int conn) {
        unique_lock<mutex>(this->_tsd_ptr->_m);
        if (this->_tsd_ptr->_match.find(conn) == this->_tsd_ptr->_match.end()) { return; }
        int oppo = this->_tsd_ptr->_match[conn];
        Buffer *rb = this->_tsd_ptr->_rbh[conn].get();
        if (!rb->endsWith("\n")) { return; }
        INFO << "receive full message " << rb->_buffer << " from " << conn << END;
        Buffer *wb = this->_tsd_ptr->_wbh[oppo].get();
        wb->appendToBuffer(rb->_buffer);
        rb->clear();
        this->writeToBuffer(oppo);
        assert(rb->writeComplete() && wb->writeComplete());
    }

    void onPassivelyClosed(int conn) {
        unique_lock<mutex>(this->_tsd_ptr->_m);
        int oppo = -1;
        INFO << "client " << conn << " closed the socket" << END;
        ::close(conn);
        if (this->_tsd_ptr->_match.find(conn) != this->_tsd_ptr->_match.cend()) {
            oppo = this->_tsd_ptr->_match[conn];
            this->_tsd_ptr->_match.erase(conn);
            ::write(oppo, Config::_end_msg, strlen(Config::_end_msg));
        } else {
            this->_tsd_ptr->_single.erase(conn);
        }
    }
};

int main() {
    ChessGameShareData data;
    EpollTPServer<ChessGameIOHandler, ChessGameShareData>
        server(8, Config::_port, &data);
    server.run();
    return 0;
}