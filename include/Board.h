#pragma once
#include <vector>
#include <string>
#include <cctype>
#include <algorithm>
#include <cassert>

// Utilities
enum class Stone { EMPTY = 0, BLACK = 1, WHITE = 2 };

Stone        opposite(Stone s);
char         stone_char(Stone s);
int          col_from_char(char ch);
char         char_from_col(int c);
std::string  trim(std::string s);

// Board class
class Board {
public:
    explicit Board(int n = 19);

    int  size() const;
    bool in_bounds(int r, int c) const;

    Stone get(int r, int c) const;
    void  set(int r, int c, Stone s);

    void clear();

    // Count stones
    void count(int& black, int& white) const;

    // Serialize
    std::string dump_rows() const;
    bool        load_rows(const std::vector<std::string>& rows);

private:
    int N;
    std::vector<Stone> grid;
};
