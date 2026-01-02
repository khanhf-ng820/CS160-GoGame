/*
TÓM TẮT SƠ
Các trường dữ liệu chính (main):
- int N: kích thước bàn (9/13/19)
- Board bd: đối tượng bàn cờ
- Stone to_move: lượt hiện tại (BLACK/WHITE)
- int consecutive_passes: đếm PASS (≥ 2 liên tiếp -> ván kết thúc) (nhưng mà có vẻ hoạt động chưa được ổn lắm !!)
- double komi: điểm bù cho trắng do trắng đi sau (tôi set là 6.5)
- vector<Move> history, redo_stack: dùng cho undo/redo.

Hàm chính:
- size(), board(), side_to_move(): đọc state cơ bản
- is_over(): true nếu có ≥ 2 lần PASS liên tiếp (check lại này giúp tôi)
- legal(m): kiểm tra biên với ô trống (có thể mở rộng thêm)
- play(m): đánh 1 nước (hoặc PASS). Ghi lịch sử, xóa redo, đổi lượt
- pass(): thêm 1 PASS, tăng consecutive_passes, đổi lượt
- undo()/redo(): quay lại/tiến tới theo lịch sử nước đi
- score(): đếm quân (BASIC), khi tính thắng thua sẽ cộng KOMI cho trắng
- serialize()/deserialize(): lưu/đọc theo tiêu đề là "GOSAVE N=.. side=.. komi=.. passes=.."
- render_ascii(): in bàn ra console
- parse_move("D4"): chuyển chuỗi tọa độ sang Move (bỏ I)
*/
#pragma once
#include <string>
#include <vector>
#include <optional>
#include <random>
#include <sstream>
#include "Board.h"

// 2 gamemodes: PvP, PvE
enum class GameMode : unsigned char { PVP, PVE };
// Game states
enum class GameState : unsigned char { PLAYING, ENDED };
// Endgame results
enum class GameResults : unsigned char { BLACK_WINS, WHITE_WINS, DRAW };

// Represent a move of row r, column c, is_pass = true if move is pass
struct Move {
    int r = 0, c = 0; bool is_pass = false;

    bool operator==(const Move& other) const {
        // You decide which members make a move "equal"
        return ((r == other.r) && (c == other.c) && !is_pass && !other.is_pass)
            || (is_pass && other.is_pass);
    }
};
// Represent the number of BLACK and WHITE stones captured
struct Captured { int black = 0, white = 0; };


// GAME CLASS controls game state and implements game logic
class Game {
public:
    // CONSTRUCTOR: Create new game with board of size 19x19
    explicit Game(int n = 19);
    // === QUERY METHODS ===
    // Returns size of board
    int size() const;
    // Returns the game's komi
    const double& komi() const;
    // Returns mutable Board for query
    Board&        board();
    // Returns const Board for query
    const Board&  board() const;
    // Returns player to move
    Stone side_to_move() const;
    // Returns true if game is ended
    bool  is_over() const;
    // Get the number of consecutive passes
    int get_consecutive_passes() const;
    // Check if game history is empty
    bool history_is_empty() const;
    // Get the size (how many boards/game states) of game history
    size_t history_size() const;
    // Get the most recent previous board in game history (ASSUMING HISTORY SIZE >= 2)
    Board get_prev_board() const;
    // Get the number of stones a player has captured
    int get_captured_stones(Stone player) const;

    // Clear the game, board, and game state
    void reset();
    // Undo a move (undo placing stone, revert turn, decrement pass counter,...)
    bool undo();
    // Redo an undone move (if move is in redo_stack)
    bool redo();
    // Pass action
    void pass();
    // Check if move is legal (move in bounds, pass move,...)
    bool legal(const Move& m) const;
    // Play the move if valid: place stone, write history, clear redo_stack, alternate turns,...
    bool play(const Move& m);

    // (ONLY USE WHEN GAME ENDS) Calculate score for both players
    void calcScore();
    // (ONLY USE WHEN GAME ENDS) Return game results (who wins or draws)
    GameResults results();
    // (ONLY USE WHEN GAME ENDS) Return player's score (black or white)
    double returnScore(Stone player);

    // Load a new board to the game (WHEN LOADING A SAVED GAME)
    void loadNewBdToHistory(Board board);

    // Returns string (as text) to save the game state into text file
    std::string serialize() const;
    // Load saved game state from text string
    bool        deserialize(const std::string& data);
    // Turn move strings: "D4", "Q11", pass, etc. into Move objects of size N
    static Move parse_move(const std::string& raw, int N);
    // Draw the board into "ASCII art" (letters A...T, rows, ...)
    std::string render_ascii() const;

    // Resign
    void resign(Stone loser);
    // Check if ended by resign
    bool ended_by_resign() const;


private:
    // Japanese komi for WHITE
    double komiPts = 6.5;

    // Game state
    GameState gameState = GameState::PLAYING;
    // Board size
    int   N;
    // Board object
    Board bd;
    // Color of player to move
    Stone to_move;
    // Consecutive passes
    int   consecutive_passes = 0;
    // Points: number of white & black stones captured by opponents
    int   blacksCaptured = 0, whitesCaptured = 0;
    // Total points of each player (territory and captures)
    double blackScore = 0, whiteScore = 0;
    // If resigned, don't calculate territory again
    bool endedByResign = false;
    // History of all boards played and all stones captured
    std::vector<Board> boardHistory;
    std::vector<Captured> captureHistory;
    // History of all moves played (for printing and keeping records ONLY)
    std::vector<Move> moveHistory;
    // Stack of undone boards and captured stones
    std::vector<Board> redo_stack;
    std::vector<Captured> capture_redoStack;


    // Clear history of boards and moves
    void clearHistory();
};
