#include <sys/socket.h>
#include <math.h>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <iostream>

#include "kingpin/Utils.h"
#include "kingpin/Buffer.h"
#include "Config.h"
#include "ChessGame.h"

using namespace std;

class Client {
public:
    char _player = 'U';

    void start() {
        int conn = initConnect(Config::_ip, Config::_port);
        Buffer buffer;
        cout << "Welcome! Now waiting for another player to join..." << endl;
        ChessGame game;
        while (true) {
            buffer.clear();
            try {
                buffer.readNioToBufferTillEnd(conn, "\n", 1);
            } catch(NonFatalException &e) {
                cout << e.what() << endl;
                break;
            }
            if (_player == 'U') {
                if (0 == ::strcmp(buffer._buffer, Config::_init_msg_red)) { _player = 'R'; }
                else { _player = 'B'; }
            }
            game.moveChess(buffer._buffer);
            game.showGameBoard();
            cout << "You are player: " << ((_player == 'R') ? "red" : "black") << endl;
            if (0 == ::strcmp(buffer._buffer, Config::_init_msg_black)) { continue; }
            string move = game.askMove();
            game.moveChess(move);
            game.showGameBoard();
            cout << "You are player: " << ((_player == 'R') ? "red" : "black") << endl;
            ::write(conn, move.c_str(), move.size());
        }
        ::close(conn);
    }
};

int main() {
    Client client;
    client.start();
    return 0;
}