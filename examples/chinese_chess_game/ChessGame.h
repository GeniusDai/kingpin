#ifndef _ChessGame_H_de485c8f1246_
#define _ChessGame_H_de485c8f1246_

#include <iostream>
#include <vector>
#include <memory>
#include <unordered_map>
#include <cstdlib>
#include <string>
#include <cassert>

using namespace std;

class ChessGame {
public:
    enum class Player { RED = 1, BLACK };

    enum class ChessType { ROOK = 1, KNIGHT, ELEPHANT, MANDARIN, KING, CANNON, PAWN };

    struct Chess {
        Chess(ChessType t, Player p) : _t(t), _p(p) {}
        ChessType _t;
        Player _p;

        struct ChessHash {
            size_t operator()(const Chess &chess) const {
                return static_cast<size_t>(chess._t) * 10 +
                    static_cast<size_t>(chess._p) / 19;
            }
        };

        bool operator==(const Chess &chess) const {
            return _t == chess._t && _p == chess._p;
        }
    };

    struct EnumHash {
        template <typename T>
        size_t operator()(const T &t) const { return static_cast<size_t>(t); }
    };

    typedef shared_ptr<Chess> _chess_ptr;

    static const string RED_COLOR;
    static const string BLACK_COLOR;
    static const int COLUMNS = 9;
    static const int ROWS = 10;
    static const unordered_map<Player, string, EnumHash> playerHash;
    static const unordered_map<Chess, string, Chess::ChessHash> chessHash;

    vector<vector<_chess_ptr> > _board;

    ChessGame() {
        _board = vector<vector<_chess_ptr> >(ROWS, vector<_chess_ptr>(COLUMNS, nullptr));
        initRows(0, Player::RED); initRows(ROWS - 1, Player::BLACK);
        initRows(2, Player::RED); initRows(ROWS - 3, Player::BLACK);
        initRows(3, Player::RED); initRows(ROWS - 4, Player::BLACK);
    }

    void initRows(int row, Player p) {
        if (row == 0 or row == ROWS - 1) {
            _board[row][0] = _chess_ptr(new Chess(ChessType::ROOK, p));
            _board[row][1] = _chess_ptr(new Chess(ChessType::KNIGHT, p));
            _board[row][2] = _chess_ptr(new Chess(ChessType::ELEPHANT, p));
            _board[row][3] = _chess_ptr(new Chess(ChessType::MANDARIN, p));
            _board[row][4] = _chess_ptr(new Chess(ChessType::KING, p));
            _board[row][COLUMNS - 4] = _chess_ptr(new Chess(ChessType::MANDARIN, p));
            _board[row][COLUMNS - 3] = _chess_ptr(new Chess(ChessType::ELEPHANT, p));
            _board[row][COLUMNS - 2] = _chess_ptr(new Chess(ChessType::KNIGHT, p));
            _board[row][COLUMNS - 1] = _chess_ptr(new Chess(ChessType::ROOK, p));
        } else if (row == 2 or row == ROWS - 3) {
            _board[row][1] = _chess_ptr(new Chess(ChessType::CANNON, p));
            _board[row][COLUMNS - 2] = _chess_ptr(new Chess(ChessType::CANNON, p));
        } else if (row == 3 or row == ROWS - 4) {
            _board[row][0] = _chess_ptr(new Chess(ChessType::PAWN, p));
            _board[row][2] = _chess_ptr(new Chess(ChessType::PAWN, p));
            _board[row][4] = _chess_ptr(new Chess(ChessType::PAWN, p));
            _board[row][COLUMNS - 3] = _chess_ptr(new Chess(ChessType::PAWN, p));
            _board[row][COLUMNS - 1] = _chess_ptr(new Chess(ChessType::PAWN, p));
        }
    }

    void showGameBoard() const {
        ::system("clear");
        for (int i = 0; i < ROWS; ++i) {
            cout << i << " ";
            for (int j = 0; j < COLUMNS; ++j) {
                if (_board[i][j] == nullptr) { cout << " " << " " << " "; }
                else {
                    cout << playerHash.at(_board[i][j]->_p) << chessHash.at(*_board[i][j]);
                    cout << "\033[0m" << " ";
                }
            }
            cout << endl;
        }
        cout << "  ";
        for (int i = 0; i < COLUMNS; ++i) { cout << i << " " << " "; }
        cout << endl;
    }

    void moveChess(int fx, int fy, int tx, int ty) {
        if (fx == tx && fy == ty) return;
        _board[tx][ty] = _board[fx][fy];
        _board[fx][fy] = nullptr;
    }

    void moveChess(string move) {
        char sep = ' ';
        vector<int> ans;
        for (size_t i = 0; i < move.size() - 1; ++i) {
            if (move[i] != sep) { ans.push_back(move[i]-'0'); }
        }
        assert(ans.size() == 4);
        moveChess(ans[0], ans[1], ans[2], ans[3]);
    }

    string askMove() {
        vector<int> ans(4);
        cout << "Enter the chess move[row column]" << endl;
        cout << "-> From : ";
        cin >> ans[0] >> ans[1];
        cout << "->  To  : ";
        cin >> ans[2] >> ans[3];
        string data;
        for (size_t i = 0; i < ans.size(); ++i) {
            data += to_string(ans[i]);
            if (i != ans.size() - 1) { data += " "; }
            else { data += "\n"; }
        }
        return data;
    }
};

const string ChessGame::RED_COLOR = "\033[47;31m";
const string ChessGame::BLACK_COLOR = "\033[47;30m";

const unordered_map<ChessGame::Player, string, ChessGame::EnumHash>
ChessGame::playerHash = {
    { ChessGame::Player::RED, ChessGame::RED_COLOR},
    { ChessGame::Player::BLACK, ChessGame::BLACK_COLOR}
};

const unordered_map<ChessGame::Chess, string, ChessGame::Chess::ChessHash>
ChessGame::chessHash = {
    {{ChessGame::ChessType::ROOK, ChessGame::Player::RED}, "???"},
    {{ChessGame::ChessType::KNIGHT, ChessGame::Player::RED}, "???"},
    {{ChessGame::ChessType::ELEPHANT, ChessGame::Player::RED}, "???"},
    {{ChessGame::ChessType::MANDARIN, ChessGame::Player::RED}, "???"},
    {{ChessGame::ChessType::KING, ChessGame::Player::RED}, "???"},
    {{ChessGame::ChessType::CANNON, ChessGame::Player::RED}, "???"},
    {{ChessGame::ChessType::PAWN, ChessGame::Player::RED}, "???"},
    {{ChessGame::ChessType::ROOK, ChessGame::Player::BLACK}, "???"},
    {{ChessGame::ChessType::KNIGHT, ChessGame::Player::BLACK}, "???"},
    {{ChessGame::ChessType::ELEPHANT, ChessGame::Player::BLACK}, "???"},
    {{ChessGame::ChessType::MANDARIN, ChessGame::Player::BLACK}, "???"},
    {{ChessGame::ChessType::KING, ChessGame::Player::BLACK}, "???"},
    {{ChessGame::ChessType::CANNON, ChessGame::Player::BLACK}, "???"},
    {{ChessGame::ChessType::PAWN, ChessGame::Player::BLACK}, "???"},
};

#endif // ChessGame.h
