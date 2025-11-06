#pragma once
#include <string>
#include <vector>
#include <optional>
#include <random>
#include <sstream>
#include "Board.h"

enum class GameMode { PVP, PVE };

struct Move { int r = 0, c = 0; bool is_pass = false; };
struct Score { int black = 0, white = 0; };

class Game {
public:
    explicit Game(int n = 19);

    int size() const;
    Board&       board();
    const Board& board() const;
    Stone side_to_move() const;

    bool  is_over() const;
    Score score() const;

    void reset();
    bool undo();
    bool redo();
    void pass();

    bool legal(const Move& m) const;
    bool play(const Move& m);

    std::string serialize() const;
    bool        deserialize(const std::string& data);

    static Move parse_move(const std::string& raw, int N);
    std::string render_ascii() const;

public:
    double komi = 6.5;

private:
    int   N;
    Board bd;
    Stone to_move;
    int   consecutive_passes = 0;

    std::vector<Move> history;
    std::vector<Move> redo_stack;
};
