#include <sys/socket.h>
#include <math.h>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <iostream>

#include "kingpin/Utils.h"
#include "Config.h"
#include "ChessGame.h"

using namespace std;

class Client {
public:
    int _conn;
    char _player = 'U';

    void start() {
        _conn = initConnect(Config::_ip, Config::_port);
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
            if (buffer[i] != Config::_init_msg[i]) {
                return false;
            }
        }
        return true;
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

int main() {
    Client client;
    client.start();
    return 0;
}