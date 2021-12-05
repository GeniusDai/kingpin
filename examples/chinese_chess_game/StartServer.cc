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
#include "kingpin/TPSharedData.h"
#include "kingpin/Exception.h"
#include "kingpin/Logger.h"
#include "kingpin/Buffer.h"

#include "Config.h"

using namespace std;

class ChessGameShareData : public ServerTPSharedData {
public:
    mutex _m;
    unordered_map<int, int> _match;
    unordered_set<int> _single;
    unordered_map<int, unique_ptr<Buffer> > _message;
};

template <typename ChessGameShareData>
class ChessGameIOHandler : public IOHandlerForServer<ChessGameShareData> {
public:
    ChessGameIOHandler(ChessGameShareData *tsd_ptr) :
        IOHandlerForServer<ChessGameShareData>(tsd_ptr) {}

    void onConnect(int conn) {
        unique_lock<mutex> lg(this->_tsd_ptr->_m);
        this->_tsd_ptr->_message[conn] = std::move(unique_ptr<Buffer>(new Buffer));
        if (this->_tsd_ptr->_single.empty()) {
            this->_tsd_ptr->_single.insert(conn);
            INFO << "single client " << conn << " arrived"<< END;
        } else {
            auto iter = this->_tsd_ptr->_single.begin();
            this->_tsd_ptr->_match[*iter] = conn;
            this->_tsd_ptr->_match[conn] = *iter;
            INFO << "match client " << conn << " & " << *iter << END;
            ::write(*iter, Config::_init_msg_red, strlen(Config::_init_msg_red));
            ::write(conn, Config::_init_msg_black, strlen(Config::_init_msg_black));
            this->_tsd_ptr->_single.erase(*iter);
        }
        this->RegisterFd(conn, EPOLLIN);
    }

    void onPassivelyClosed(int conn) {
        INFO << "client closed the socket, will close " << conn << END;
        ::close(conn);
        this->_tsd_ptr->_message.erase(conn);
        if (this->_tsd_ptr->_match.find(conn) != this->_tsd_ptr->_match.cend()) {
            INFO << "find oppo, will close socket " << this->_tsd_ptr->_match[conn] << END;
            ::close(this->_tsd_ptr->_match[conn]);
            this->_tsd_ptr->_message.erase(this->_tsd_ptr->_match[conn]);
            this->_tsd_ptr->_match.erase(this->_tsd_ptr->_match[conn]);
        } else {
            this->_tsd_ptr->_single.erase(conn);
        }
        this->_tsd_ptr->_match.erase(conn);
    }

    void onReadable(int conn, uint32_t events) {
        unique_lock<mutex> lg(this->_tsd_ptr->_m);
        Buffer *p_buf = this->_tsd_ptr->_message[conn].get();
        try {
            p_buf->readNioToBufferTillBlock(conn, 100);
            if (p_buf->endsWith("\n")) {
                INFO << "receive full message " << p_buf->_buffer << " from " << conn << END;
                p_buf->writeNioFromBuffer(this->_tsd_ptr->_match[conn]);
            }
        } catch (NonFatalException &e) {
            INFO << e << END;
            onPassivelyClosed(conn);
            return;
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