#pragma once
#include <vector>
#include <string>
#include <cctype>
#include <algorithm>
#include <cassert>

// ULTILITIES
// Define a class to represent the coordinates of an intersection
struct Intersection { int row = 0, col = 0; Intersection(int r, int c): row(r), col(c) {};};
// Define enum class Stone to represent state of an intersection / a player (EMPTY means no player)
enum class Stone { EMPTY = 0, BLACK = 1, WHITE = 2 };
// Define enum class Liberty to denote if an intersection has liberty or not (EMPTY means intersection with NO STONES)
// enum class Liberty { EMPTY = 0, HAS_LIBERTY = 1, NO_LIBERTY = 2 };
// Returns the opposite stone color (Black -> White, White -> Black)
Stone        opposite(Stone s);
// Returns char representing an intersection ('X': BLACK, 'O': WHITE, '.': EMPTY)
char         stone_char(Stone s);
// Turns column char (A, B, C, ..., T, ignore I due to Go standards) into column int
int          col_from_char(char ch);
// Opposite of the function above
char         char_from_col(int c);
// Trim whitespaces at the start and end of string (For playing using CLI, reading save file, etc.)
std::string  trim(std::string s);

// CONSTS
// Signify which intersection coordinates are considered 'adjacent'
const std::vector<Intersection> offsets = { {-1, 0}, {1, 0}, {0, -1}, {0, 1} };

// BOARD CLASS
class Board {
public:
    // Constructor creates board of size 19x19, 'explicit' helps compiler differentiate between 19 <-> Board(19) => Board is int (Cause BUGS)
    explicit Board(int n = 19);
    // Returns the size N of the board: number of intersections on one side (For query purposes)
    int  size() const;
    // Returns the grid vector<Stone>
    std::vector<Stone> getGrid() const;
    // Returns 1D index of 2D coordinates (r, c): r * N + c
    int  idx1D(int r, int c) const;
    // Check if (r, c) is inside the board
    bool in_bounds(int r, int c) const;
    // Get the coordinates of an intersection's neighbors (stones adjacent to it)
    std::vector<Intersection> getNeighbors(int r, int c) const;
    // Get intersection state at (r, c)
    Stone get(int r, int c) const;
    // Place Stone s at (r, c), applies board and stone logic in Go
    // Rule: Prohibition of suicide (capturing one's own stones). Function checks if it's suicide move
    // Returns true if it's non-suicide move, false if it's suicide move
    bool  set(int r, int c, Stone s);
    // Clear the board, reset all vectors to EMPTY
    void clear();
    // Count BLACK and WHITE stones on board
    void count(int& black, int& white) const;
    // Count if an intersection is adjacent how many black or white stone(s), or empty intersection(s) (liberty)
    int interNearStone(int r, int c, Stone stone) const;
    // Returns a vector of all stones of a player that will be captured (removed) due to no liberties
    std::vector<Intersection> toBeCaptured(Stone player);

    // Count the number of intersections in a player's territory (ONLY USED FOR SCORING WHEN GAME ENDS)
    int countTerritory(Stone player) const;

    // COMPARE BOARDS
    bool operator==(const Board& board2) const;
    // Count how many stones were captured AFTER playing a move
    int countCaptured(const Board& previousBoard, Stone played) const;

    // SERIALIZE
    // Returns string (as text) to save the board state into text file
    std::string dump_rows() const;
    // Load board state from string (when loading saved game)
    bool        load_rows(const std::vector<std::string>& rows);


protected:
    // Size of the board (N * N)
    int N;
    // 1D vector to save the board's state: Each element is an intersection, with 3 states: BLACK, WHITE, EMPTY
    std::vector<Stone> grid;

private:
    // 1D vector to denote how many liberties an intersection has
    std::vector<int> libertyCount;

    // Recursive DFS algorithm function to put all intersections of 'stone' from (r, c) into 'components'
    void dfs(int r, int c, Stone stone, std::vector<Intersection>& components, std::vector<bool>& visited) const;
    // Check for each intersection, if it has liberty or not, and output them to the 'hasLiberty' vector
    void checkLiberty();
};
