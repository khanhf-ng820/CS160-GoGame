#include "AI_Utils/GamePosition.h"



// ***** Class for a Game State *****
// Constructor
GamePosition::GamePosition(Board bd, Stone to_move, const double& komi) : Board(bd.size()), prevBoard(Board(bd.size())), to_move(to_move), komi(komi) {
    grid = bd.getGrid();
    libertyCount = bd.getLibertyCnt();
}
GamePosition::GamePosition(const Game& game) : Board(game.board().size()), prevBoard(Board(game.board().size())), to_move(game.side_to_move()), komi(game.komi()), consecutive_passes(game.get_consecutive_passes()), blacksCaptured(game.get_captured_stones(Stone::WHITE)), whitesCaptured(game.get_captured_stones(Stone::BLACK)) {
    grid = game.board().getGrid();
    Board bd = game.board(); bd.checkLiberty();
    libertyCount = bd.getLibertyCnt();
    if (!game.history_is_empty()) {
        prevBoardIsNull = false;
        prevBoard = game.get_prev_board();
    }
}
// Get the board from a game state (game position) pointer
Board GamePosition::getBd(const GamePosition* game_state) const { return * (Board*) game_state; }
// Get the color of player to-move
Stone GamePosition::getPlayerToMove() const { return to_move; }
// Get the number of consecutive passes
int GamePosition::getConsPasses() const { return consecutive_passes; }
// Increment the number of consecutive passes
void GamePosition::incConsPasses() { consecutive_passes++; }
// Check if game ends (>= 2 consecutive passes)
bool GamePosition::gameEnded() const { return consecutive_passes >= 2; }
// Get the previous board
Board GamePosition::getPrevBoard() const { return prevBoard; }
// Count how many a player's stones have been captured
int GamePosition::getCaptured(Stone player) const { return ((player == Stone::BLACK) ? blacksCaptured : ((player == Stone::WHITE) ? whitesCaptured : 0)); }

// === PRIVATE METHODS ===
// Examine 2x2 window on board to check if it's Q1, Q3, Qd pattern
bool GamePosition::isQ1(Stone a[4], Stone player) const {
    return (a[0] == player && a[1] == Stone::EMPTY && a[2] == Stone::EMPTY && a[3] == Stone::EMPTY)
        || (a[0] == Stone::EMPTY && a[1] == player && a[2] == Stone::EMPTY && a[3] == Stone::EMPTY)
        || (a[0] == Stone::EMPTY && a[1] == Stone::EMPTY && a[2] == player && a[3] == Stone::EMPTY)
        || (a[0] == Stone::EMPTY && a[1] == Stone::EMPTY && a[2] == Stone::EMPTY && a[3] == player);
}
bool GamePosition::isQ3(Stone a[4], Stone player) const {
    return (a[0] == Stone::EMPTY && a[1] == player && a[2] == player && a[3] == player)
        || (a[0] == player && a[1] == Stone::EMPTY && a[2] == player && a[3] == player)
        || (a[0] == player && a[1] == player && a[2] == Stone::EMPTY && a[3] == player)
        || (a[0] == player && a[1] == player && a[2] == player && a[3] == Stone::EMPTY);
}
bool GamePosition::isQd(Stone a[4], Stone player) const {
    return (a[0] == Stone::EMPTY && a[1] == player && a[2] == player && a[3] == Stone::EMPTY)
        || (a[0] == player && a[1] == Stone::EMPTY && a[2] == Stone::EMPTY && a[3] == player);
}
// =======================

// Check legality of move
bool GamePosition::legal(const Move& mv) const {
    // Can't play if game is ended
    if (gameEnded()) return false;
    // Pass is always legal
    if (mv.is_pass) return true;
    // Out of bounds -> illegal
    if (!in_bounds(mv.r, mv.c)) return false;
    // Intersection must be EMPTY
    if (get(mv.r, mv.c) != Stone::EMPTY) return false;

    // Rule: Prohibition of suicide (capturing one's own stones)
    Board futureBoard = *this;
    // Check if any of own's stones will be captured
    if (!futureBoard.set(mv.r, mv.c, to_move)) return false;

    // Ko rule: One may not play in such a way as to recreate the board position
    // following one's previous move.
    if (prevBoardIsNull) return true;
    if (prevBoard == futureBoard) return false;

    return true; // Move is legal
}

// Play move if legal
bool GamePosition::play(const Move& m) {
    // Reject move if illegal
    if (!legal(m)) return false;
    // If it's pass move, call pass function, check if game ends, return true
    if (m.is_pass) {
        // Set the previous board to the current board
        prevBoardIsNull = false;
        prevBoard = *this;
        consecutive_passes++; // Increment consecutive_passes
        to_move = opposite(to_move); // Alternate turns
        
        return true; // Valid move
    }

    // Set prevBoard equal to the current board
    prevBoardIsNull = false;
    prevBoard = *this;
    // Place player's stone at intersection with coordinates (r, c), resolve all game and board logic
    set(m.r, m.c, to_move);
    // Always update liberties
    checkLiberty();
    
    // Add points based on how many stones were captured
    if (to_move == Stone::BLACK) {
        whitesCaptured += countCaptured(prevBoard, to_move);
    } else if (to_move == Stone::WHITE) {
        blacksCaptured += countCaptured(prevBoard, to_move);
    }
    
    // A "placing stone" move with clear PASS counter
    consecutive_passes = 0;
    // Alternate turns
    to_move = opposite(to_move);
    
    return true;
}

// Check if a move will capture stone(s) if played
bool GamePosition::isCapture(const Move& mv) const {
    // Reject move if illegal / pass move
    if (!legal(mv) || mv.is_pass) return false;

    // Check if any adjacent enemy stone is in ATARI
    for (const auto& [r_offset, c_offset] : _OFFSETS_) {
        int r0 = mv.r + r_offset, c0 = mv.c + c_offset;
        if (libertyCount[idx1D(r0, c0)] == 1)
            return true;
    }
    return false;
}

// Count how many stones will be captured after playing a move
int GamePosition::willCapture(const Move& mv) const {
    // Reject move if illegal / pass move
    if (!legal(mv) || mv.is_pass) return 0;

    // Simulate move
    GamePosition futureState = *this;
    futureState.play(mv);
    switch (to_move) {
    case Stone::WHITE:
        return futureState.blacksCaptured - blacksCaptured;
    case Stone::BLACK:
        return futureState.whitesCaptured - whitesCaptured;
    default:
        return 0;
    }
}

// Count how many liberties each individual stone has, and sum up
void GamePosition::countIndividualLiberty(int& black, int& white) const {
    black = white = 0;
    for (int r = 0; r < N; r++) {
        for (int c = 0; c < N; c++) {
            if (get(r, c) == Stone::BLACK) {
                black += interNearStone(r, c, Stone::EMPTY);
            } else if (get(r, c) == Stone::WHITE) {
                white += interNearStone(r, c, Stone::EMPTY);
            }
        }
    }
}

// Count how many stones in atari each player has
void GamePosition::countAtari(int& black, int& white) const {
    black = white = 0;
    GamePosition cloneState = *this;
    cloneState.checkLiberty();
    for (int r = 0; r < N; r++) {
        for (int c = 0; c < N; c++) {
            if (cloneState.libertyCount[idx1D(r, c)] == 1) {
                if (get(r, c) == Stone::BLACK)
                    black++;
                else if (get(r, c) == Stone::WHITE)
                    white++;
            }
        }
    }
}

// Calculate the Euler number of the board with respect to each individual player
void GamePosition::calcEuler(double& black, double& white) const {
    int q1b = 0, q3b = 0, qdb = 0, q1w = 0, q3w = 0, qdw = 0;
    for (int r = 0; r < N-1; r++) {
        for (int c = 0; c < N-1; c++) {
            Stone boardWindow[4] = {
                get(r, c), get(r, c+1),
                get(r+1, c), get(r+1, c+1)
            };
            // For black
            q1b += isQ1(boardWindow, Stone::BLACK);
            q3b += isQ3(boardWindow, Stone::BLACK);
            qdb += isQd(boardWindow, Stone::BLACK);
            // For white
            q1w += isQ1(boardWindow, Stone::WHITE);
            q3w += isQ3(boardWindow, Stone::WHITE);
            qdw += isQd(boardWindow, Stone::WHITE);
        }
    }
    black = (q1b - q3b + 2 * qdb) / 4.0;
    white = (q1w - q3w + 2 * qdw) / 4.0;
}

// Return the game score for a player
double GamePosition::calcScore(Stone player) const {
    switch (player) {
    case Stone::WHITE:
        return countTerritory(player) + blacksCaptured + komi;
    case Stone::BLACK:
        return countTerritory(player) + whitesCaptured;
    default:
        return 0;
    }
}
