#include <sys/socket.h>
#include <math.h>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <iostream>
#include "ChessGame.h"

using namespace std;

#ifndef _Client_
#define _Client_

class Client {
public:
    const char *_initMsg = "0 0 0 0\n";
    const int _port = 8889;
    const char *_ip = "127.0.0.1";
    int _conn;
    char _player = 'U';

    void start() {
        connectServer();
        char buffer[100];
        int len = 100;
        memset(buffer, 0, 100*sizeof(char));
        cout << "Welcome! Now waiting for another player to join..." << endl;
        ChessGame game;
        while (true) {
            if (readConn(_conn, buffer, &len) < 0) {
                cout << "Socket closed" << endl;
                break;
            }
            if (_player == 'U') {
                if (isInitMsg(buffer, &len)) {
                    _player = 'R';
                } else {
                    _player = 'B';
                }
            }

            string move = "";
            for (int i = 0; i < len; ++i) {
                move += buffer[i];
            }
            game.moveChess(move);

            game.showGameBoard();
            cout << "You are player: " << ((_player == 'R') ? "red" : "black") << endl;
            move = game.askMove();
            game.moveChess(move);
            game.showGameBoard();
            cout << "You are player: " << ((_player == 'R') ? "red" : "black") << endl;
            move += '\n';
            write(_conn, move.c_str(), move.size());
        }
        ::close(_conn);
    }

    bool isInitMsg(const char *buffer, const int *len) {
        for (int i = 0; i < *len; ++i) {
            if (buffer[i] != _initMsg[i]) {
                return false;
            }
        }
        return true;
    }

    void connectServer() {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == -1) {
            throw "socket error";
        }
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_port = htons(_port);
        addr.sin_family = AF_INET;
        inet_pton(AF_INET, _ip, &addr.sin_addr.s_addr);
        if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            throw "connect error";
        }
        cout << "connected to server" << endl;
        this->_conn = sock;
    }

    int readConn(int conn, char *buffer, int *len) {
        int size = 0;
        int curr;
        while(true) {
            curr = ::read(conn, buffer+size, *len);
            if (curr == 0) {
                return -1;
            } else if (curr < 0) {
                throw "read error";
            }
            this_thread::sleep_for(chrono::milliseconds(100));
            size += curr;
            if (buffer[size-1] == '\n') {
                break;
            }
        }
        *len = size;
        return 0;
    }
};

#endif // Client.h
