#pragma once
#include <iostream>
#include <iomanip>
#include <limits>
#include <vector>
#include <queue>
#include <algorithm>
#include <random>
#include "Board.h"
#include "Game.h"

// Constants
constexpr double _POSITIVE_INF_ = 10000;
constexpr double _NEGATIVE_INF_ = -10000;
constexpr double _EPSILON_ = 1e-9;
constexpr Move   _PASS_MOVE_ = {0,0,true};

// Structs for easier storing
struct Branch {
    Move move; double evaluation;
    Branch(Move mv, double eval): move(mv), evaluation(eval) {};
};

// Enum class for AI difficulty
enum class AIDifficulty { EASY, MEDIUM, HARD };

// ***** Class for a Game State (Game position) *****
class GamePosition : public Board {
public:
    // Constructor
    explicit GamePosition(Board bd, Stone to_move, double komi);
    explicit GamePosition(const Game& game);
    // Get the board from a game state (game position) pointer
    Board getBd(const GamePosition* game_state) const;
    // Get the color of player to-move
    Stone getPlayerToMove() const;
    // Get the number of consecutive passes
    int getConsPasses() const;
    // Increment the number of consecutive passes
    void incConsPasses();
    // Check if game ends (>= 2 consecutive passes)
    bool gameEnded() const;
    // Get the previous board
    Board getPrevBoard() const;

    // Check if move is legal
    bool legal(const Move& mv) const;
    // Play a move
    bool play(const Move& mv);
    // Return the game score for a player
    double calcScore(Stone player) const;

    // Count how many stones were captured after playing a move
    // int countCaptured(const GamePosition& previousBoard, Stone played) const;


private:
    // Default Japanese komi
    double komi = 6.5;
    // Color of player to-move
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



// ***** Class for ZOBRIST HASHES *****
// class ZobristHash {
// public:
//     // Constructor
//     explicit ZobristHash(); // Generate a random one
//     explicit ZobristHash(uint64_t hashValue);
//     // Get the hash
//     uint64_t getHash() const;
//     // XOR operator
//     ZobristHash operator^(const ZobristHash& rightOperand) const;

// private:
//     uint64_t hash;
// };

// // Generating bitstrings for each possible element of the game
// uint64_t generateRandom64() {
//     // static ensures the engine is initialized only once.
//     // thread_local makes this safe for multi-threaded use.
//     thread_local static std::mt19937_64 gen(std::random_device{}());
    
//     // Distribution for the full 64-bit range [0, 2^64 - 1]
//     static std::uniform_int_distribution<uint64_t> dis;
    
//     return dis(gen);
// }



// BLACK is MAXimizing player
// WHITE is MINimizing player
// ***** Class for the Go AI opponent *****
class GoAI {
public:
    explicit GoAI(AIDifficulty d=AIDifficulty::EASY);
    // explicit GoAI(AIDifficulty d=AIDifficulty::EASY, std::mt19937& rng);

    AIDifficulty getDiff() const;

    // Choose move depending on difficulty
    Move choose_move(const Game& game, std::mt19937& rng);


private:
    // AI difficulty
    AIDifficulty diff;
    // Depth limits for minimax algorithm
    unsigned int medium_search_depth = 2;
    unsigned int hard_search_depth = 3;

    // Limit for Manhattan distance for MEDIUM/HARD modes (must be around 2 or 3)
    int          maxManDistMedium = 3;
    int          maxCheDistHard = 2;
    // Limit of branching factor
    unsigned int branchingFactorLimit = 15;
    // Number of killer moves stored in AI
    unsigned int killerMoveStorage = 2;
    // Storing the killer moves (FOR ALPHA-BETA PRUNING)
    std::vector<std::vector<Move>> killerMoves = std::vector<std::vector<Move>>(hard_search_depth, std::vector<Move>());

    // Get Manhattan distance between two points on the board
    int manhattan_dist(int r0, int c0, int r1, int c1);


    // ***** FOR MINIMAX ALGORITHM *****
    // Terminal function (when game ends)
    bool terminal(const GamePosition& game_state);
    // Action function
    // Returns RANDOMLY SHUFFLED vector of ALL legal moves possible from that position, including pass
    std::vector<Move> action(const GamePosition& game_state, std::mt19937& rng);
    // Action functions that returns the MOST REASONABLE moves (for MEDIUM/HARD modes)
    // === These moves are considered REASONABLE moves: ===
    // 1. Influence: Only consider empty points within Manhattan distance <= 2-3 of any stone
    // 2. (HARD MODE ONLY) Influence: Only consider empty points within Manhattan distance <= 3 of the most recently placed stone
    std::vector<Move> reasonableActions(const GamePosition& game_state, std::mt19937& rng);
    // Result function (new game state from old game state + a legal move/pass)
    GamePosition result(const GamePosition& game_state, const Move& move);

    // Evaluation function (heuristic function) for Go board (range: [-1, 1])
    double evaluate(const GamePosition& game_state);

    // Minimax value functions
    double max_value(const GamePosition& game_state, unsigned int depth, std::mt19937& rng);
    double min_value(const GamePosition& game_state, unsigned int depth, std::mt19937& rng);

    // Minimax value function with depth-limited minimax, NO ALPHA-BETA PRUNING
    double naive_minimax(const GamePosition& game_state, unsigned int depth, Stone player, std::mt19937& rng);
    // Minimax value function with depth-limited minimax, WITH ALPHA-BETA PRUNING
    double alpha_beta(const GamePosition& game_state, unsigned int depth, double alpha, double beta, Stone player, std::mt19937& rng);


    // Choose move for three difficulties
    Move choose_move_easy(const Game& game, std::mt19937& rng);
    Move choose_move_medium(const Game& game, std::mt19937& rng);
    Move choose_move_hard(const Game& game, std::mt19937& rng);
};
