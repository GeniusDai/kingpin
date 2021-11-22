#include <iostream>
#include <vector>
#include <memory>
#include <unordered_map>
#include <cstdlib>
#include <string>
#include <cassert>

using namespace std;

#ifdef _DEBUG
#undef _DEBUG
#endif

#define _DEBUG

#ifndef _ChessGame_
#define _ChessGame_

class ChessGame {
public:

    enum class Player{
        RED = 1,
        BLACK,
    };

    enum class ChessType {
        ROOK = 1,
        KNIGHT,
        ELEPHANT,
        MANDARIN,
        KING,
        CANNON,
        PAWN,
    };

    struct Chess {
        Chess(ChessType t, Player p) : t_(t), p_(p) {}
        ChessType t_;
        Player p_;
        bool operator==(const Chess &chess) const {
            return t_ == chess.t_ && p_ == chess.p_;
        }
    };

    struct ChessHash {
        size_t operator()(const Chess &chess) const {
            return (static_cast<size_t>(chess.t_) * 10 + static_cast<size_t>(chess.p_)) / 19;
        }
    };

    struct EnumHash {
        template <typename T>
        size_t operator()(const T &t) const {
            return static_cast<size_t>(t);
        }
    };

    ChessGame() {
        initHash();
        board_ = vector<vector<_chess_ptr> >(ROWS, vector<_chess_ptr>(COLUMNS, nullptr));
        initRows(0, Player::RED); initRows(ROWS - 1, Player::BLACK);
        initRows(2, Player::RED); initRows(ROWS - 3, Player::BLACK);
        initRows(3, Player::RED); initRows(ROWS - 4, Player::BLACK);
    }

    void initHash() {

        playerHash[Player::RED] = RED_COLOR;
        playerHash[Player::BLACK] = BLACK_COLOR;

        chessHash[{ChessType::ROOK, Player::RED}] = "车";
        chessHash[{ChessType::KNIGHT, Player::RED}] = "马";
        chessHash[{ChessType::ELEPHANT, Player::RED}] = "象";
        chessHash[{ChessType::MANDARIN, Player::RED}] = "士";
        chessHash[{ChessType::KING, Player::RED}] = "帅";
        chessHash[{ChessType::CANNON, Player::RED}] = "炮";
        chessHash[{ChessType::PAWN, Player::RED}] = "兵";

        chessHash[{ChessType::ROOK, Player::BLACK}] = "车";
        chessHash[{ChessType::KNIGHT, Player::BLACK}] = "马";
        chessHash[{ChessType::ELEPHANT, Player::BLACK}] = "象";
        chessHash[{ChessType::MANDARIN, Player::BLACK}] = "仕";
        chessHash[{ChessType::KING, Player::BLACK}] = "将";
        chessHash[{ChessType::CANNON, Player::BLACK}] = "炮";
        chessHash[{ChessType::PAWN, Player::BLACK}] = "卒";
    }

    void initRows(int row, Player p) {
        if (row == 0 or row == ROWS - 1) {
            board_[row][0] = _chess_ptr(new Chess(ChessType::ROOK, p));
            board_[row][1] = _chess_ptr(new Chess(ChessType::KNIGHT, p));
            board_[row][2] = _chess_ptr(new Chess(ChessType::ELEPHANT, p));
            board_[row][3] = _chess_ptr(new Chess(ChessType::MANDARIN, p));
            board_[row][4] = _chess_ptr(new Chess(ChessType::KING, p));
            board_[row][COLUMNS - 4] = _chess_ptr(new Chess(ChessType::MANDARIN, p));
            board_[row][COLUMNS - 3] = _chess_ptr(new Chess(ChessType::ELEPHANT, p));
            board_[row][COLUMNS - 2] = _chess_ptr(new Chess(ChessType::KNIGHT, p));
            board_[row][COLUMNS - 1] = _chess_ptr(new Chess(ChessType::ROOK, p));
        } else if (row == 2 or row == ROWS - 3) {
            board_[row][1] = _chess_ptr(new Chess(ChessType::CANNON, p));
            board_[row][COLUMNS - 2] = _chess_ptr(new Chess(ChessType::CANNON, p));
        } else if (row == 3 or row == ROWS - 4) {
            board_[row][0] = _chess_ptr(new Chess(ChessType::PAWN, p));
            board_[row][2] = _chess_ptr(new Chess(ChessType::PAWN, p));
            board_[row][4] = _chess_ptr(new Chess(ChessType::PAWN, p));
            board_[row][COLUMNS - 3] = _chess_ptr(new Chess(ChessType::PAWN, p));
            board_[row][COLUMNS - 1] = _chess_ptr(new Chess(ChessType::PAWN, p));
        }
    }

    void showGameBoard() {
        ::system("clear");
        for (int i = 0; i < ROWS; ++i) {
            cout << i << " ";
            for (int j = 0; j < COLUMNS; ++j) {
                _chess_ptr tmpChess(board_[i][j]);
                if (tmpChess == nullptr) {
                    cout << " " << " " << " ";
                } else {
                    cout << playerHash[tmpChess->p_]<< chessHash[*tmpChess];
                    cout << "\033[0m" << " ";
                }
            }
            cout << endl;
        }
        cout << "  ";
        for (int i = 0; i < COLUMNS; ++i) {
            cout << i << " " << " ";
        }
        cout << endl;
    }

    void moveChess(int fx, int fy, int tx, int ty) {
        if (fx == tx && fy == ty) return;
        board_[tx][ty] = board_[fx][fy];
        board_[fx][fy] = _chess_ptr(nullptr);
    }

    void moveChess(vector<int> &ans) {
        moveChess(ans[0], ans[1], ans[2], ans[3]);
    }

    void moveChess(string move) {
        char sep = ' ';
        move += sep;
        int begin = -1;
        vector<int> ans;
        for (int i = 0; i < move.size(); ++i) {
            if (move[i] != sep) {
                if (begin < 0) begin = i;
            } else {
                if (begin >= 0) {
                    ans.push_back(stoi(move.substr(begin, i - begin)));
                    begin = -1;
                }
            }
        }
        assert(ans.size() == 4);
        moveChess(ans);
    }

    string askMove() {
        vector<int> ans(4);
        cout << "Enter the chess move[row column]" << endl;
        cout << "-> From : ";
        cin >> ans[0] >> ans[1];
        cout << "->  To  : ";
        cin >> ans[2] >> ans[3];
        string data;
        for (int i = 0; i < ans.size(); ++i) {
            data += to_string(ans[i]);
            if (i != ans.size() - 1) data += " ";
        }
        return data;
    }

private:
    static const string RED_COLOR;
    static const string BLACK_COLOR;
    typedef shared_ptr<Chess> _chess_ptr;
    vector<vector<_chess_ptr> > board_;
    static const int COLUMNS = 9;
    static const int ROWS = 10;
    unordered_map<Player, string, EnumHash> playerHash;
    unordered_map<Chess, string, ChessHash> chessHash;
};

const string ChessGame::RED_COLOR = "\033[47;31m";
const string ChessGame::BLACK_COLOR = "\033[47;30m";

#endif // ChessGame.h
