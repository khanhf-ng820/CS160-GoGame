/*
TÓM TẮT TRƯỚC CÁC HÀM
- opposite(s): trả màu còn lại
- stone_char(s): '.'/'X'/'O' để xuất ra ASCII
+ ***** (X: BLACK, O: WHITE) *****
- col_from_char(ch): chuyển 'A'..'T' -> 0..(N-1). Input có thể là chữ thường, sẽ có chuẩn hoá thành chữ hoa
- char_from_col(c): 0..N-1 -> 'A'..'T'
- trim(s): xóa khoảng trống 2 đầu (lệnh này bên UI xài nhiều hơn, nó viết bên Board vậy thôi)
- ctor Board(int n): tạo bảng N×N. Mặc định là EMPTY
- in_bounds(r,c): kiểm tra vị trí quân cờ
- get/set: đọc/ghi ô
- clear(): đưa toàn bộ ô về EMPTY
- count(black, white): đếm số quây, đây mới cơ bản thôi chứ nếu muốn đếm TERRITORY thì sẽ cần thêm luật bổ sung
- dump_rows(): gom N dòng ký tự cho save
- load_rows(rows): load lại bàn từ N chuỗi, bỏ qua ký tự lạ, chỉ nhận .XO
*/
#include "Board.h"

// ULTILITIES
Stone opposite(Stone s) {
// Returns the opposite stone color (Black -> White, White -> Black)
// *** NOTE: EMPTY still returns BLACK to make it easier to alternate turns ***
    return (s == Stone::BLACK ? Stone::WHITE : Stone::BLACK);
}

// Returns char representing an intersection ('X': BLACK, 'O': WHITE, '.': EMPTY)
char stone_char(Stone s) {
    if (s == Stone::BLACK) return 'X';
    if (s == Stone::WHITE) return 'O';
    return '.';
}

// Turns column char (A, B, C, ..., T, ignore I due to standards) into column int
int col_from_char(char ch) {
    // Turns ch into uppercase
    if (ch >= 'a' && ch <= 'z') ch = char(ch - 'a' + 'A');
    // If not alphabet letter, returns -1 (Invalid value)
    if (!(ch >= 'A' && ch <= 'Z')) return -1;
    // Ignore 'I' due to Go standards
    // If ch ≥ 'J', subtract 1
    if (ch >= 'J') return (ch - 'A') - 1;
    // If ch in range (AB..H) just return ch - 'A'
    return ch - 'A';
}

// Opposite of the function above
char char_from_col(int c) {
    char col = char('A' + c);
    if (col >= 'I') col++;
    return col;
}

// Trim whitespaces at the start and end of string (For playing using CLI, reading save file, etc.)
std::string trim(std::string s) {
    // Index a starts from beginning of string, b starts from end of string
    size_t a = 0, b = s.size();
    // Increase a, decrease b, delete whitespaces
    while (a < b && std::isspace((unsigned char)s[a])) ++a;
    while (b > a && std::isspace((unsigned char)s[b - 1])) --b;
    // Returns substring [a, b)
    return s.substr(a, b - a);
}



// ********************
// BOARD IMPLEMENTATION
// ********************
// Constructor saves size N of board, creates 1D vectors (grid, libertyCount) of size NxN, full of EMPTY
Board::Board(int n) : N(n), grid(n * n, Stone::EMPTY), libertyCount(n * n, 0) {}

// Returns the size N of the board: number of intersections on one side (For query purposes)
int Board::size() const { return N; }

// Returns the grid vector<Stone>
std::vector<Stone> Board::getGrid() const { return grid; }

// Returns 1D index of 2D coordinates (r, c)
int Board::idx1D(int r, int c) const { return c + N * r; }

// Check if (r, c) is inside the board
bool Board::in_bounds(int r, int c) const {
    // Valid nếu 0 ≤ r, c ≤ N
    return r >= 0 && r < N && c >= 0 && c < N;
}

// Get the coordinates of an intersection's neighbors (stones adjacent to it)
std::vector<Intersection> Board::getNeighbors(int r, int c) const {
    std::vector<Intersection> neighbors;

    for (const auto& [dx, dy] : offsets) {
        if (in_bounds(r + dx, c + dy)) {
            neighbors.push_back(Intersection(r + dx, c + dy));
        }
    }
    return neighbors;
}

// Get intersection state at (r, c)
Stone Board::get(int r, int c) const {
    // Check if valid, in-bound coordinates
    assert(in_bounds(r, c));
    // Turn 2D coordinates -> 1D coordinates
    return grid[r * N + c];
}

// Place Stone s at (r, c), applies board and stone logic in Go
// Rule: Prohibition of suicide (capturing own stones). Function checks if it's suicide move
// Returns true if it's non-suicide move, false if it's suicide move
bool Board::set(int r, int c, Stone s) {
    // Check if valid, in-bound coordinates
    assert(in_bounds(r, c));

    // Step 1: Playing a stone
    grid[r * N + c] = s;
    // Step 2: Capture opponent's stones with no liberties
    std::vector<Intersection> opponentCaptured = toBeCaptured(opposite(s));
    for (const auto& [r0, c0] : opponentCaptured)  grid[idx1D(r0, c0)] = Stone::EMPTY;
    // Step 3: Capture own stones with no liberties
    std::vector<Intersection> ownCaptured = toBeCaptured(s);
    for (const auto& [r0, c0] : ownCaptured)  grid[idx1D(r0, c0)] = Stone::EMPTY;

    // Prohibition of suicide: Check if any of own's stones will be captured
    if (ownCaptured.size() > 0) return false;
    
    return true; // Valid, non-suicide move
}

// Clear the board, reset all vectors to EMPTY
void Board::clear() {
    // std::fill EMPTY values
    std::fill(grid.begin(), grid.end(), Stone::EMPTY);
    std::fill(libertyCount.begin(), libertyCount.end(), 0);
}

// Count BLACK and WHITE stones on board
void Board::count(int& black, int& white) const {
    // Reset counter to 0
    black = white = 0;
    // Loop through grid
    for (auto s : grid) {
        if (s == Stone::BLACK) ++black;
        else if (s == Stone::WHITE) ++white;
    }
}

// Check if an intersection is adjacent to how many black or white stone(s), or empty intersection(s) (liberty)
int Board::interNearStone(int r, int c, Stone stone) const {
    std::vector<Intersection> neighbors = getNeighbors(r, c);
    int counter = 0;
    for (const auto& [r0, c0] : neighbors) {
        if (get(r0, c0) == stone) counter++;
    }
    return counter;
}

// PRIVATE member of Board class
// Recursive DFS algorithm function for Board::checkLiberty() member function
void Board::dfs(int r, int c, Stone stone, std::vector<Intersection>& components, std::vector<bool>& visited) const {
    int idx = idx1D(r, c);
    if (visited[idx] || get(r, c) != stone) return;
    visited[idx] = true;
    components.push_back(Intersection(r, c));

    std::vector<Intersection> neighbors = getNeighbors(r, c);
    for (const auto& [r0, c0] : neighbors) {
        if (get(r0, c0) == stone) dfs(r0, c0, stone, components, visited);
    }
}

// Check liberties of all intersections and output them to the libertyCount vector
void Board::checkLiberty() {
    // Perform DFS to find all components of intersections of the same type
    std::vector<bool> visited(N * N, false);

    for (int r = 0; r < N; r++) {
        for (int c = 0; c < N; c++) {
            std::vector<Intersection> components;
            Stone componentStone = get(r, c);
            dfs(r, c, componentStone, components, visited); // Perform DFS

            int libertyCounter = 0;
            // Check each intersection of component whether it has liberty
            for (const auto& [r0, c0] : components) {
                int libs = interNearStone(r0, c0, Stone::EMPTY);
                if (libs) {
                    libertyCounter += libs;
                }
            }

            // Output results to the libertyCount vector
            for (const auto& [r0, c0] : components) {
                int idx0 = idx1D(r0, c0);
                // We only care about liberties of stones, not liberties of an empty space
                if (componentStone == Stone::EMPTY) {
                    libertyCount[idx0] = 0;
                } else {
                    libertyCount[idx0] = libertyCounter;
                }
            }
        }
    }
}

// Returns a vector of all stones of a player that will be captured (removed) due to no liberties
std::vector<Intersection> Board::toBeCaptured(Stone player) {
    checkLiberty();
    std::vector<Intersection> noLiberties;

    for (int r = 0; r < N; r++) {
        for (int c = 0; c < N; c++) {
            int idx = idx1D(r, c);
            // Check if stone has no liberties and belongs to player
            if (libertyCount[idx] == 0 && get(r, c) == player)
                noLiberties.push_back(Intersection(r, c));
        }
    }

    return noLiberties;
}

// Count the number of intersections in a player's territory (ONLY USED FOR SCORING WHEN GAME ENDS)
int Board::countTerritory(Stone player) const {
    if (player == Stone::EMPTY) return 0;
    // *** Perform DFS to find all components of empty intersections
    int territoryCounter = 0;
    std::vector<bool> visited(N * N, false);

    for (int r = 0; r < N; r++) {
        for (int c = 0; c < N; c++) {
            std::vector<Intersection> components;
            dfs(r, c, Stone::EMPTY, components, visited); // Perform DFS

            bool inBlackTerritory = false, inWhiteTerritory = false;
            // Check each empty intersection of component whether it's near a black stone
            for (const auto& [r0, c0] : components) {
                if (interNearStone(r0, c0, Stone::BLACK)) {
                    inBlackTerritory = true;
                    break;
                }
            }
            // Check each empty intersection of component whether it's near a white stone
            for (const auto& [r0, c0] : components) {
                if (interNearStone(r0, c0, Stone::WHITE)) {
                    inWhiteTerritory = true;
                    break;
                }
            }

            // Check if in player's territory and add to counter
            if ((player == Stone::BLACK && inBlackTerritory  && !inWhiteTerritory) || 
                (player == Stone::WHITE && !inBlackTerritory && inWhiteTerritory )) {
                territoryCounter += components.size();
            }
        }
    }
    return territoryCounter;
}



// Compare boards
bool Board::operator==(const Board& board2) const {
    return grid == board2.getGrid();
}

// Count how many stones were captured after playing a move
int Board::countCaptured(const Board& previousBoard, Stone played) const {
    // Count black & white stones in previous board
    int prevBlack = 0, prevWhite = 0;
    previousBoard.count(prevBlack, prevWhite);
    // Count black & white stones in current board
    int curBlack = 0, curWhite = 0;
    count(curBlack, curWhite);

    // Count how many stones were captured
    if (played == Stone::BLACK) {
        return prevWhite - curWhite;
    } else if (played == Stone::WHITE) {
        return prevBlack - curBlack;
    }
    return 0;
}



// *** SERIALIZE ***
// Returns string (as text) to save the board state into text file
std::string Board::dump_rows() const {
    // Reserve N lines, each line has N chars + 1 newline char
    std::string out; out.reserve(N * (N + 1));
    // Loop through rows
    for (int r = 0; r < N; ++r) {
        // Loop through columns
        for (int c = 0; c < N; ++c)
            // Add intersection char (./X/O)
            out.push_back(stone_char(get(r, c)));
        // Add newline
        out.push_back('\n');
    }
    return out;
}

// Load board state from string (when loading saved game)
bool Board::load_rows(const std::vector<std::string>& rows) {
    // Check if exactly N lines
    if ((int)rows.size() != N) return false;
    // Loop through lines
    for (int r = 0; r < N; ++r) {
        // Each line must have exactly N chars
        if ((int)rows[r].size() != N) return false;
        // Loop through columns in row
        for (int c = 0; c < N; ++c) {
            char ch = rows[r][c];
            Stone s = Stone::EMPTY;
            if (ch == 'X') s = Stone::BLACK;
            else if (ch == 'O') s = Stone::WHITE;
            // Set stone back to board
            // set(r, c, s);
            grid[r * N + c] = s;
        }
    }
    return true; // Everything valid
}
