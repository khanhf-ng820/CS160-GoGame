/*
GIẢI THÍCH FLOW
class Game:
1) legal(m):
- Nếu m.is_pass thì hợp lệ
- Ngược lại thì kiểm tra in_bounds với ô đang EMPTY
(Mở rộng thêm luật như cấm tự sát,...)

2) play(m):
- Nếu PASS: gọi pass() và return true
- Nếu đánh quân thường:
    +) Đặt quân: bd.set(r,c,to_move)
    +) push vào history với clear redo_stack
    +) reset consecutive_passes = 0
    +) Đổi lượt: to_move = opposite(to_move)
- Trả false nếu m không hợp lệ

3) pass():
- push Move{is_pass=true} vào history với xóa redo_stack
- ++consecutive_passes + đổi lượt
- is_over() -> true khi consecutive_passes ≥ 2

4) undo()/redo():
- undo: pop history -> nếu là PASS thì giảm counter còn nếu là đặt quân thì clear ô
        Sau đó đổi lượt và push move đó sang redo_stack
- redo: lấy lại từ redo_stack và gọi play(m)

5) serialize()/deserialize():
- Đầu tiên là ghi header: "GOSAVE N=.. side=.. komi=.. passes=.."
- deserialize sẽ đọc cả 2 dạng

6) score():
- Dùng bd.count(black, white). Khi quyết định thắng thua, TRẮNG + komi

7) parse_move("D4"):
- Bỏ chữ i/I, chấp nhận lower/upper

LƯU Ý
- Các utilities từ Board.h: trim, col_from_char/char_from_col, stone_char
*/
#include "Game.h"
#include <iostream>
#include <algorithm> // Use std::max when parsing header,...

// GAME CLASS controls game state and implements game logic
// Create new game with board of size n x n
// Initialize N = n, create empty Board NxN, BLACK moves first
Game::Game(int n) : N(n), bd(n), to_move(Stone::BLACK), boardHistory(1, Board(n)), captureHistory(1, {0,0}) {}
// Returns size of board
int Game::size() const { return N; }
// Returns the game's komi
double Game::komi() const { return komiPts; }
// Returns mutable Board for query (method is not const)
Board& Game::board() { return bd; }
// Returns const Board for query (const method)
const Board& Game::board() const { return bd; }
// Returns player to move
Stone Game::side_to_move() const { return to_move; }
// Returns true if game is ended (Game ends if 2 or more consecutive passes)
bool Game::is_over() const { return consecutive_passes >= 2; }

// Clear the game, board, and game state
void Game::reset() {
    // Game state set to Playing
    gameState = GameState::PLAYING;
    // Clear board, set BLACK to move
    bd.clear(); to_move = Stone::BLACK;
    // Clear history and redo_stack
    boardHistory.clear();
    captureHistory.clear();
    moveHistory.clear();
    redo_stack.clear();
    capture_redoStack.clear();
    // Clear pass counter
    consecutive_passes = 0;

    blacksCaptured = whitesCaptured = 0;
    blackScore = whiteScore = 0;
    endedByResign = false;
}

// Resign
bool Game::ended_by_resign() const {
    return endedByResign;
}

// Undo a move (if possible) (undo placing stone, revert turn, decrement pass counter,...)
bool Game::undo() {
    // Can't undo if game is ended
    if (gameState == GameState::ENDED) return false;
    // No moves to undo
    if (boardHistory.size() <= 1) return false;

    // // Lấy nước cuối cùng khỏi history
    // Move mv = boardHistory.back();
    // boardHistory.pop_back();
    // // Với pass: chỉ đảo lượt lại
    // if (mv.is_pass) {
    //     to_move = opposite(to_move);
    //     // Giảm số pass đã đến liên tiếp nếu > 0
    //     if (consecutive_passes > 0) --consecutive_passes;
    // } else {
    //     // Gỡ quân vừa đặt ở (r, c)
    //     bd.set(mv.r, mv.c, Stone::EMPTY);
    //     // Đảo lượt lại cho bên vừa đi
    //     to_move = opposite(to_move);
    //     // Một nước đặt quân sẽ reset chuỗi pass
    //     consecutive_passes = 0;
    // }
    // // Đưa nước vừa hoàn tác vào redo_stack để có thể redo
    // redo_stack.push_back(mv);
    // return true;

    // Get last board state from history
    Board previousBd = boardHistory[boardHistory.size() - 2];
    // Revert the number of BLACK and WHITE stones captured
    Captured capturedStones = captureHistory[captureHistory.size() - 2];
    blacksCaptured = capturedStones.black;
    whitesCaptured = capturedStones.white;
    // If pass move: Just alternate the turn
    if (previousBd == bd) {
        to_move = opposite(to_move);
        // Decrease consecutive pass counter
        if (consecutive_passes > 0) --consecutive_passes;
    } else {
        bd = previousBd;
        // Alternate turn
        to_move = opposite(to_move);
        // Reset consecutive pass counter (because it's "placing stone" move)
        consecutive_passes = 0;
    }
    // Add undone move to redo_stack to be redone later
    Board undoneBoard = boardHistory.back();
    boardHistory.pop_back();
    redo_stack.push_back(undoneBoard);
    // Add undone captures to capture_redoStack to be redone later
    Captured undoneCaptures = captureHistory.back();
    captureHistory.pop_back();
    capture_redoStack.push_back(undoneCaptures);
    return true;
}

// Redo 1 undone move (if possible)
bool Game::redo() {
    // Can't redo if game is ended
    if (gameState == GameState::ENDED) return false;
    // No undone moves to redo
    if (redo_stack.empty()) return false;

    // // Lấy nước từ redo_stack và bỏ khỏi stack
    // Move mv = redo_stack.back();
    // redo_stack.pop_back();
    // // Áp dụng lại logic của play để làm lại nước đi
    // return play(mv);

    // Pop board state from redo_stack
    Board redoneBoard = redo_stack.back();
    redo_stack.pop_back();
    // Pop the number of BLACK and WHITE stones captured from capture_redoStack
    Captured capturedStones = capture_redoStack.back();
    capture_redoStack.pop_back();

    // Alternate turns
    to_move = opposite(to_move);
    // If pass move: set consecutive_passes to 1.
    // Else: set consecutive_passes to 0  (because it's "placing stone" move). 
    consecutive_passes = (bd == redoneBoard);

    // Push redone move to game history
    boardHistory.push_back(redoneBoard);
    bd = redoneBoard;
    // Push the number of BLACK and WHITE stones captured to captureHistory
    captureHistory.push_back(capturedStones);
    blacksCaptured = capturedStones.black;
    whitesCaptured = capturedStones.white;
    return true;
}

// Do pass action
void Game::pass() {
    // Can't pass if game is ended
    if (gameState == GameState::ENDED) return;
    // Set previousBd equal to the current board
    Board previousBd = bd;
    // Push to board history and move history where move.is_pass = true
    boardHistory.push_back(bd);
    captureHistory.push_back( {blacksCaptured, whitesCaptured} );
    moveHistory.push_back( Move{0,0,true} );
    // When playing new move (even pass), redo stacks is cleared
    redo_stack.clear();
    capture_redoStack.clear();
    // Increment consecutive_passes counter
    ++consecutive_passes;
    // Alternate turns
    to_move = opposite(to_move);
}

// Check legality of move
bool Game::legal(const Move& m) const {
    // Can't play if game is ended
    if (gameState == GameState::ENDED) return false;
    // Pass is always legal
    if (m.is_pass) return true;
    // Out of bounds -> illegal
    if (!bd.in_bounds(m.r, m.c)) return false;
    // Intersection must be EMPTY
    if (bd.get(m.r, m.c) != Stone::EMPTY) return false;

    // Rule: Prohibition of suicide (capturing one's own stones)
    Board futureBoard = bd;
    // Check if any of own's stones will be captured
    if (!futureBoard.set(m.r, m.c, to_move)) return false;

    // Ko rule: One may not play in such a way as to recreate the board position
    // following one's previous move.
    Board prevBoard = (boardHistory.size() > 1) ? boardHistory[boardHistory.size() - 2] : Board(N);
    if (prevBoard == futureBoard) return false;

    return true; // Move is legal
}

// Play move if legal
bool Game::play(const Move& m) {
    // Reject move if illegal
    if (!legal(m)) return false;
    // If it's pass move, call pass function, check if game ends, return true
    if (m.is_pass) {
        pass();
        // If 2 consecutive passes, GAME ENDS
        if (is_over()) {
            gameState = GameState::ENDED;
        }
        return true; // Valid move
    }

    // Set previousBd equal to the current board
    Board previousBd = bd;
    // Place player's stone at intersection with coordinates (r, c), resolve all game and board logic
    bd.set(m.r, m.c, to_move);
    
    // Add points based on how many stones were captured
    int capturedStones = bd.countCaptured(previousBd, to_move);
    if (to_move == Stone::BLACK) {
        whitesCaptured += capturedStones;
    } else if (to_move == Stone::WHITE) {
        blacksCaptured += capturedStones;
    }
    
    // Write to history to be able to undo
    boardHistory.push_back(bd);
    captureHistory.push_back( {blacksCaptured, whitesCaptured} );
    moveHistory.push_back(m);
    // Done new move -> Can't redo anything
    redo_stack.clear();
    capture_redoStack.clear();
    // A "placing stone" move with clear PASS counter
    consecutive_passes = 0;
    // Alternate turns
    to_move = opposite(to_move);
    
    return true;
}

// (ONLY USE WHEN GAME ENDS) Calculate score for both players
void Game::calcScore() {
    if (endedByResign) return;
    int blackTerritory = bd.countTerritory(Stone::BLACK);
    int whiteTerritory = bd.countTerritory(Stone::WHITE);
    std::cout << "[SCORE] Black territory: " << blackTerritory << std::endl;
    std::cout << "[SCORE] White territory: " << whiteTerritory << std::endl;
    std::cout << "[SCORE] White stones captured by black: " << whitesCaptured << std::endl;
    std::cout << "[SCORE] Black stones captured by white: " << blacksCaptured << std::endl;
    blackScore = blackTerritory + whitesCaptured;
    whiteScore = whiteTerritory + blacksCaptured;
    whiteScore += komiPts; // Komi for white
    std::cout << "[SCORE] Black total score: " << blackScore << std::endl;
    std::cout << "[SCORE] White total score: " << whiteScore << std::endl;
}

// (ONLY USE WHEN GAME ENDS) Return game results (who wins or draws)
GameResults Game::results() {
    calcScore(); // Calculate score before returning game results
    if (blackScore > whiteScore) {
        return GameResults::BLACK_WINS;
    } else if (blackScore < whiteScore) {
        return GameResults::WHITE_WINS;
    } else {
        return GameResults::DRAW;
    }
}

// (ONLY USE WHEN GAME ENDS) Return player's score (black or white)
double Game::returnScore(Stone player) {
    calcScore(); // Calculate score before returning game results
    if (player == Stone::BLACK) {
        return blackScore;
    } else if (player == Stone::WHITE) {
        return whiteScore;
    } else {
        return -1;
    }
}

// Clear history of boards and moves
void Game::clearHistory() {
    boardHistory.clear();
    captureHistory.clear();
    moveHistory.clear();
    redo_stack.clear();
    capture_redoStack.clear();
    gameState = GameState::PLAYING;
}

// Load a new board to the game (when loading a saved game)
void Game::loadNewBdToHistory(Board board) {
    clearHistory();
    boardHistory.push_back(board);
}



// Returns string (as text) to save the game state into text file
std::string Game::serialize() const {
    // Create output string stream
    std::ostringstream oss;
    oss << "GOSAVE N=" << N
        // Encode player-to-move (Black = 0, White = 1)
        << " side=" << (to_move == Stone::BLACK ? 0 : 1)
        // Write komi points
        << " komi=" << komiPts
        // Write number of consecutive PASSES
        << " passes=" << consecutive_passes
        // Write number of BLACK stones captured
        << " blacksCaptured=" << blacksCaptured
        // Write number of WHITE stones captured
        << " whitesCaptured=" << whitesCaptured
        << "\n";
    // Write the board matrix (N lines, N chars on each line)
    oss << bd.dump_rows();
    // Return the result string
    return oss.str();
}

// Load saved game state from text string
bool Game::deserialize(const std::string& data) {
    // Create input string stream to read line-by-line
    std::istringstream iss(data);
    // First line (header or board)
    std::string first;
    // Return false (error) if can't read first line
    if (!std::getline(iss, first)) return false;

    if (first.size() >= 3 &&
        (unsigned char)first[0] == 0xEF &&
        (unsigned char)first[1] == 0xBB &&
        (unsigned char)first[2] == 0xBF)
        first.erase(0, 3);
    // Remove Byte Order Mark (BOM UTF-8)

    // Lambda function to clear '\r' chars at end of line for compatibility
    auto rstrip_cr = [](std::string& s){
        if (!s.empty() && s.back() == '\r') s.pop_back();
    };

    auto only_rows = [&](std::string head)->bool{
        // Clear '\r' chars at the end
        rstrip_cr(head);
        // First line must have at least N chars (less than N -> invalid)
        if ((int)head.size() < N) return false;

        // Create capacity N rows vector to represent the board
        std::vector<std::string> rows; rows.reserve(N);
        // Only take the first N chars for row 0
        rows.push_back(head.substr(0, N));

        while ((int)rows.size() < N) {
            std::string line;
            // Make sure to read exactly N lines, if fewer -> error
            if (!std::getline(iss, line)) return false;
            // Clear '\r' chars at the end
            rstrip_cr(line);
            // If line has fewer than N chars, skip to next one
            if ((int)line.size() < N) continue;
            // Only take the first N chars for next row
            rows.push_back(line.substr(0, N));
        }

        // Create empty board with size N
        bd = Board(N);
        // BLACK moves first by default
        to_move = Stone::BLACK;
        // Default consecutive_passes set to 0
        consecutive_passes = 0;
        // Load rows string into Board object
        return bd.load_rows(rows);
    };

    // In case the save file doesn't have GOSAVE HEADER and only have matrix with N rows
    // If first line contains string "GO SAVE" -> HEADER exists
    if (first.rfind("GOSAVE", 0) == 0) {
        // Trim whitespace chars
        std::string header = trim(first);
        // Default values of N, side-to-move, komi and consecutive passes
        int n = 19, side = 0, passes = 0; double k = 6.5;

        // Take lambda function
        auto take = [&](const std::string& key)->std::optional<std::string>{
            auto pos = header.find(key);
            if (pos == std::string::npos) return std::nullopt;
            pos += key.size();
            size_t e = header.find_first_of(" \t", pos);
            if (e == std::string::npos) e = header.size();
            return header.substr(pos, e - pos);
        };
        // Fetch important data: "N=", "side=", "komi=", "passes=" from HEADER
        // Get N and clamp N above 1 ( >= 1 )
        if (auto s = take("N="))      n = std::max(1, std::stoi(*s));
        // Get the current player to move (0 = Black, 1 = White)
        if (auto s = take("side="))   side = std::stoi(*s);
        // Get komi
        if (auto s = take("komi="))   k = std::stod(*s);
        // Get number of consecutive PASSES and clamp above 0 ( >= 0 )
        if (auto s = take("passes=")) passes = std::max(0, std::stoi(*s));
        // Get the # of BLACK stones captured
        if (auto s = take("blacksCaptured="))   blacksCaptured = std::max(0, std::stoi(*s));
        // Get the # of WHITE stones captured
        if (auto s = take("whitesCaptured="))   whitesCaptured = std::max(0, std::stoi(*s));

        // Update size N and create new board of size N
        N = n; bd = Board(N);
        // Update komi value
        komiPts = k;
        // Update side-to-move value
        to_move = (side == 0 ? Stone::BLACK : Stone::WHITE);
        // Update number of consecutive PASSES
        consecutive_passes = passes;

        // Read the N-row matrix
        std::vector<std::string> rows; rows.reserve(N);
        while ((int)rows.size() < N) {
            std::string line;
            // If fewer than N rows, return false (error)
            if (!std::getline(iss, line)) return false;
            // Strip '\r' chars
            rstrip_cr(line);
            // Skip all lines of fewer than N chars
            if ((int)line.size() < N) continue;
            // Only take the first N chars for each line
            rows.push_back(line.substr(0, N));
        }
        // Check if loading rows is successful
        bool loadRowsOK = bd.load_rows(rows);
        // Clear history and load new board into game history
        if (loadRowsOK) loadNewBdToHistory(bd);
        // Loaded into 'board' and return boolean value as result
        return loadRowsOK;
    } else {
        // Check if loading rows is successful
        bool loadOnlyRowsOK = only_rows(first);
        // Clear history and load new board into game history
        if (loadOnlyRowsOK) loadNewBdToHistory(bd);
        // If HEADER doesn't exist, then deserialize savefile with matrix only
        return loadOnlyRowsOK;
    }
}

// Turn move strings: "D4", "Q11", pass, etc. into Move objects of size N
Move Game::parse_move(const std::string& raw, int N) {
    // Trim whitespace chars
    std::string s = trim(raw);
    // Make letters uppercase
    for (char& ch : s) if (ch >= 'a' && ch <= 'z') ch = char(ch - 'a' + 'A');
    // PASS / RESIGN are considered PASS moves
    if (s == "PASS" || s == "RESIGN") return {0,0,true};
    // If it's too short to be considered a move string, make it a PASS move
    if (s.size() < 2) return {0,0,true};
    // Turn column (A...T) into index
    int c = col_from_char(s[0]);
    // Convert the integer number to be the row index
    int r = std::stoi(s.substr(1)) - 1;
    // If row/column out of bounds, consider it a PASS move
    if (c < 0 || r < 0 || r >= N) return {0,0,true};
    // Valid -> return Move object with is_pass = false
    return {r, c, false};
}

// Draw the board into "ASCII art" (letters A...T, rows, ...)
std::string Game::render_ascii() const {
    // Result buffer
    std::ostringstream oss;
    // The column labels at the top
    oss << "   ";
    // Print A..T with spaces
    for (int c = 0; c < N; ++c) oss << char_from_col(c) << ' ';
    // New line after the column top labels
    oss << "\n";

    for (int r = 0; r < N; ++r) {
        // Row labels: Numbers decreasing from N to 1
        int rowLabel = N - r;
        // Add left padding for 1-digit numbers
        if (rowLabel < 10) oss << ' ';
        // Print row labels on the left
        oss << rowLabel << ' ';
        // Print each intersection of the board: '.', 'X', 'O' with spaces
        for (int c = 0; c < N; ++c)
            oss << stone_char(bd.get(r, c)) << ' ';
        // Print row labels on the right, with newline
        oss << ' ' << rowLabel << '\n';
    }

    // The column labels at the bottom
    oss << "   ";
    // Print A..T with spaces
    for (int c = 0; c < N; ++c) oss << char_from_col(c) << ' ';
    // End with newline
    oss << "\n";
    // Return board ASCII string
    return oss.str();
}

// Resign
void Game::resign(Stone loser) {
    gameState = GameState::ENDED;
    endedByResign = true;
    if (loser == Stone::BLACK) {
        blackScore = 0;
        whiteScore = 10000;
    } else if (loser == Stone::WHITE) {
        whiteScore = 0;
        blackScore = 10000;
    }
}
