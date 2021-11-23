#ifndef _RAM_HANDLER_H_
#define _RAM_HANDLER_H_

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

#include "kingpin/core/IOHandlerServer.h"
#include "kingpin/core/ThreadShareData.h"

using namespace std;

class ChessGameShareData : public ThreadShareDataServer {
public:
    unordered_map<int, int> match;
    unordered_set<int> single;
    unordered_map<int, pair<unique_ptr<char []>, int> > message;
};

template <typename ChessGameShareData>
class ChessGameIOHandler : public IOHandlerServer<ChessGameShareData> {
    static const int _msgBufferSize = 100;
    const char *initMsg = "0 0 0 0\n";
public:
    ChessGameIOHandler(ChessGameShareData *tsd_ptr) : IOHandlerServer<ChessGameShareData>(tsd_ptr) {}

    void onConnect(int conn) {
        this->_tsd_ptr->message[conn] = make_pair(unique_ptr<char []>(new char[_msgBufferSize]), 0);
        if (this->_tsd_ptr->single.empty()) {
            this->_tsd_ptr->single.insert(conn);
            cout << "single client " << conn << endl;
        } else {
            auto iter = this->_tsd_ptr->single.begin();
            int oppo = *iter;
            this->_tsd_ptr->single.erase(iter);
            this->_tsd_ptr->match[oppo] = conn;
            this->_tsd_ptr->match[conn] = oppo;
            write(oppo, initMsg, strlen(initMsg));
            cout << "match client " << conn << "&" << oppo << endl;
        }
        this->RegisterFd(conn, EPOLLIN);
    }

    void onReadable(int conn, uint32_t events) {
        char buf[100];
        int len = read(conn, buf, 100);
        if (len == 0) {
            cout << "client closed the socket, will close " << conn << endl;
            ::close(conn);
            this->RemoveFd(conn);
            this->_tsd_ptr->message.erase(conn);
            if (this->_tsd_ptr->match.find(conn) != this->_tsd_ptr->match.cend()) {
                cout << "find oppo, will close socket " << this->_tsd_ptr->match[conn] << endl;
                ::close(this->_tsd_ptr->match[conn]);
                this->RemoveFd(this->_tsd_ptr->match[conn]);
                this->_tsd_ptr->message.erase(this->_tsd_ptr->match[conn]);
                this->_tsd_ptr->match.erase(this->_tsd_ptr->match[conn]);
            } else {
                this->_tsd_ptr->single.erase(conn);
            }
            this->_tsd_ptr->match.erase(conn);
            return;
        }

        _appendOrSendMessage(conn, buf, len);
    }

    void _appendOrSendMessage(int conn, char *buf, int len) {
        int curr = this->_tsd_ptr->message[conn].second;
        if (len + curr >= _msgBufferSize) {
            throw "buffer overflow error";
        }
        for (int i = 0; i < len; ++i) {
            this->_tsd_ptr->message[conn].first[curr+i] = buf[i];
        }
        this->_tsd_ptr->message[conn].second = curr + len;
        if (this->_tsd_ptr->message[conn].first[curr+len-1] == '\n') {
            cout << "receive message from " << conn << endl;
            ::write(this->_tsd_ptr->match[conn], this->_tsd_ptr->message[conn].first.get(), this->_tsd_ptr->message[conn].second);
            this->_tsd_ptr->message[conn].second = 0;
        } else {
            cout << "receive partial message from " << conn << endl;
        }
    }

    void onWritable(int conn, uint32_t events) {

    }
};

#endif  // Server.h
