#include "Board.h"

// Utilities
Stone opposite(Stone s) {
    return (s == Stone::BLACK ? Stone::WHITE : Stone::BLACK);
}

char stone_char(Stone s) {
    if (s == Stone::BLACK) return 'X';
    if (s == Stone::WHITE) return 'O';
    return '.';
}

int col_from_char(char ch) {
    if (ch >= 'a' && ch <= 'z') ch = char(ch - 'a' + 'A');
    if (!(ch >= 'A' && ch <= 'Z')) return -1;
    if (ch >= 'J') return (ch - 'A') - 1; // bỏ I
    return ch - 'A';
}

char char_from_col(int c) {
    char col = char('A' + c);
    if (col >= 'I') col++;
    return col;
}

std::string trim(std::string s) {
    size_t a = 0, b = s.size();
    while (a < b && std::isspace((unsigned char)s[a])) ++a;
    while (b > a && std::isspace((unsigned char)s[b - 1])) --b;
    return s.substr(a, b - a);
}

// Board implementation
Board::Board(int n) : N(n), grid(n * n, Stone::EMPTY) {}

int Board::size() const { return N; }

bool Board::in_bounds(int r, int c) const {
    return r >= 0 && r < N && c >= 0 && c < N;
}

Stone Board::get(int r, int c) const {
    assert(in_bounds(r, c));
    return grid[r * N + c];
}

void Board::set(int r, int c, Stone s) {
    assert(in_bounds(r, c));
    grid[r * N + c] = s;
}

void Board::clear() {
    std::fill(grid.begin(), grid.end(), Stone::EMPTY);
}

void Board::count(int& black, int& white) const {
    black = white = 0;
    for (auto s : grid) {
        if (s == Stone::BLACK) ++black;
        else if (s == Stone::WHITE) ++white;
    }
}

std::string Board::dump_rows() const {
    std::string out; out.reserve(N * (N + 1));
    for (int r = 0; r < N; ++r) {
        for (int c = 0; c < N; ++c)
            out.push_back(stone_char(get(r, c)));
        out.push_back('\n');
    }
    return out;
}

bool Board::load_rows(const std::vector<std::string>& rows) {
    if ((int)rows.size() != N) return false;
    for (int r = 0; r < N; ++r) {
        if ((int)rows[r].size() != N) return false;
        for (int c = 0; c < N; ++c) {
            char ch = rows[r][c];
            Stone s = Stone::EMPTY;
            if (ch == 'X') s = Stone::BLACK;
            else if (ch == 'O') s = Stone::WHITE;
            set(r, c, s);
        }
    }
    return true;
}
