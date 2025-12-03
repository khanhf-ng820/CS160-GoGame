#pragma once
#include <random>
#include "Board.h"
#include "Game.h"

enum class AIDifficulty { EASY, MEDIUM, HARD };

// ***** Class for a Game State (Game position) *****
class GamePosition : public Board {
public:
    // Constructor
    explicit GamePosition(Board bd, Stone to_move);
    // Get the board from a game state (game position)
    Board* getBd(const GamePosition* game_state) const;
    // Get the number of consecutive passes
    int getConsPasses() const;
    // Increment the number of consecutive passes
    void incConsPasses();
    // Check if game ends (>= 2 consecutive passes)
    bool gameEnded() const;

    // Check if move is legal
    bool legal(const Move& mv) const;
    // Play a move
    bool play(const Move& mv);

    // Count how many stones were captured after playing a move
    // int countCaptured(const GamePosition& previousBoard, Stone played) const;


private:
    // Default Japanese komi
    double komi = 6.5;
    // Color of player to move
    Stone to_move;
    // Number of consecutive passes
    int consecutive_passes = 0;
    // The previous board
    Board prevBoard;
    // If the previous board is null, meaning it's the very beginning of the game
    bool prevBoardIsNull = true;
    // Points: number of white & black stones captured by opponents
    int blacksCaptured = 0, whitesCaptured = 0;
};



// BLACK is MAXimizing player
// WHITE is MINimizing player
// ***** Class for the Go AI opponent *****
class GoAI {
public:
    explicit GoAI(AIDifficulty d=AIDifficulty::EASY);

    AIDifficulty getDiff() const;

    // ***** For minimax algorithm *****
    // Terminal function (when game ends)
    bool terminal(const GamePosition& game_state);
    // Action function (get all legal moves possible from that position, including pass)
    std::vector<Move> action(const GamePosition& game_state);
    // Result function (new game state from old game state + a move)
    GamePosition result(const GamePosition& game_state, const Move& move);

    // Heuristic function for Go board
    double heuristic(const GamePosition& game_state);

    // Minimax value functions
    double max_value(GamePosition game_state, int depth);
    double min_value(GamePosition game_state, int depth);

    // Choose move depending on difficulty
    Move choose_move(const Game& game, std::mt19937& rng);

    // For three difficulties
    Move choose_move_easy(const Game& game, std::mt19937& rng);
    Move choose_move_medium(const Game& game, std::mt19937& rng);
    Move choose_move_hard(const Game& game, std::mt19937& rng);

private:
    AIDifficulty diff;
    int medium_search_depth = 2;
    int hard_search_depth = 3;
};
