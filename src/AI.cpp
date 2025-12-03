#include "AI.h"
#include <vector>
#include <algorithm>


// ***** Class for a Game State *****
// Constructor
GamePosition::GamePosition(Board bd, Stone to_move) : Board(bd.size()), prevBoard(Board(bd.size())), to_move(to_move) {
    grid = bd.getGrid();
}
// Get the board from a game state (game position)
Board* GamePosition::getBd(const GamePosition* game_state) const { return (Board*) game_state; }
// Get the number of consecutive passes
int GamePosition::getConsPasses() const { return consecutive_passes; }
// Increment the number of consecutive passes
void GamePosition::incConsPasses() { consecutive_passes++; }
// Check if game ends (>= 2 consecutive passes)
bool GamePosition::gameEnded() const { return consecutive_passes >= 2; }

// Check legality of move
bool GamePosition::legal(const Move& mv) const {
    // Can't play if game is ended
    if (gameEnded()) return false;
    // Pass is always legal
    if (mv.is_pass) return true;
    // Out of bounds -> illegal
    if (!in_bounds(mv.r, mv.c)) return false;
    // Intersection must be EMPTY
    if (this->get(mv.r, mv.c) != Stone::EMPTY) return false;

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







// ***** Class for the Go AI *****
GoAI::GoAI(AIDifficulty d): diff(d) {}
AIDifficulty GoAI::getDiff() const { return diff; }





Move GoAI::choose_move(const Game& game, std::mt19937& rng){
    const int N = game.size();
    // std::vector<Move> cand; cand.reserve(N*N+1);

    // for (int r = 0; r < N; r++)
    //     for (int c = 0; c < N; c++)
    //         if (game.board().get(r,c)==Stone::EMPTY)
    //             cand.push_back( {r,c,false} );

    // if (cand.empty()) return {0,0,true}; // pass

    // if (diff == AIDifficulty::EASY){
    //     std::uniform_int_distribution<int> D(0, (int)cand.size() - 1);
    //     return cand[D(rng)];
    // } else {
    //     const float cr = (N-1)*0.5f, cc = (N-1)*0.5f;
    //     auto sc = [&](const Move& m){ float dr=m.r-cr, dc=m.c-cc; return dr*dr+dc*dc; };
    //     std::sort(cand.begin(), cand.end(),
    //               [&](auto& a,auto& b){ return sc(a)<sc(b); });
    //     size_t k = (diff==AIDifficulty::HARD)
    //                ? std::max<size_t>(1, cand.size()/33)
    //                : std::max<size_t>(1, cand.size()/10);
    //     std::uniform_int_distribution<size_t> D(0,k-1);
    //     return cand[D(rng)];
    // }

    switch (diff) {
    case AIDifficulty::EASY:
        return choose_move_easy(game, rng);
    case AIDifficulty::MEDIUM:
        return choose_move_easy(game, rng);
    case AIDifficulty::HARD:
        return choose_move_easy(game, rng);
    }
    return choose_move_easy(game, rng);
}

// For EASY MODE
Move GoAI::choose_move_easy(const Game& game, std::mt19937& rng) {
    const int N = game.size();
    std::vector<Move> legalMoves; legalMoves.reserve(N * N + 1);

    // Consider all possible legal moves
    for (int r = 0; r < N; r++) {
        for (int c = 0; c < N; c++) {
            Move mv = {r,c,false};
            if (game.board().get(r, c) == Stone::EMPTY && game.legal(mv))
                legalMoves.push_back(mv);
        }
    }
    legalMoves.push_back( {0,0,true} ); // Pass

    // Pick a random move
    std::uniform_int_distribution<int> distrib(0, (int)legalMoves.size() - 1);
    return legalMoves[distrib(rng)];
}

// For MEDIUM MODE
Move GoAI::choose_move_medium(const Game& game, std::mt19937& rng) {
    return {0,0,true}; // ***** PLACEHOLDER, WILL DELETE LATER *****
}
