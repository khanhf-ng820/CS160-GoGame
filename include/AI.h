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
constexpr double _POSITIVE_INF_ = 100000;
constexpr double _NEGATIVE_INF_ = -100000;
constexpr double _EPSILON_ = 1e-9;
constexpr Move   _PASS_MOVE_ = {0,0,true};
constexpr size_t _MAX_PLIES_ = 1024;

// Structs for easier storing
struct Branch {
    Move move; int eval;
    Branch(Move mv): move(mv), eval(0) {};
    Branch(Move mv, int eval): move(mv), eval(eval) {};
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
    // Count how many a player's stones have been captured
    int getCaptured(Stone player) const;

    // Check if move is legal
    bool legal(const Move& mv) const;
    // Play a move
    bool play(const Move& mv);
    // Check if a move will capture stone(s) if played
    bool isCapture(const Move& mv) const;
    // Count how many stones will be captured after playing a move
    int willCapture(const Move& mv) const;
    // Count how many liberties each individual stone has, and sum up
    void countIndividualLiberty(int& black, int& white) const;
    // Count how many stones in atari each player has
    void countAtari(int& black, int& white) const;
    // Calculate the Euler number of the board with respect to each individual player
    void calcEuler(double& black, double& white) const;
    // Return the game score for a player
    double calcScore(Stone player) const;


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

    // Examine 2x2 window on board to check if it's Q1, Q3, Qd pattern
    bool isQ1(Stone a[4], Stone player) const;
    bool isQ3(Stone a[4], Stone player) const;
    bool isQd(Stone a[4], Stone player) const;
};





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

    // Limit for Manhattan/Chebyshev distance for MEDIUM/HARD modes (must be around 2 or 3)
    int          maxManDistMedium = 3;
    int          maxCheDistHard = 3;
    // Limit of branching factor
    unsigned int branchingFactorLimit = 25;
    // Number of killer moves stored in AI (commonly set to 2)
    static const unsigned int killerMoveStorage = 2;
    // Storing the killer moves FOR ALPHA-BETA PRUNING (each ply has a primary and secondary killer move)
    std::vector<std::vector<Move>> killerMoves = std::vector<std::vector<Move>>(_MAX_PLIES_, std::vector<Move>());

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
    // 2. (HARD MODE ONLY) Influence: Only consider empty points within Manhattan distance <= 3 of the most recently placed stone, and then take the best k moves
    // ===== Action function for MEDIUM MODE ONLY =====
    std::vector<Move> reasonableActionsMedium(const GamePosition& game_state, std::mt19937& rng);
    // ===== Action function for HARD MODE ONLY =====
    std::vector<Move> reasonableActionsHard(const GamePosition& game_state, unsigned int ply, std::mt19937& rng);
    // Result function (new game state from old game state + a legal move/pass)
    GamePosition result(const GamePosition& game_state, const Move& move);

    // Cheap evaluation function ONLY for MOVE ORDERING
    int    cheap_evaluate(const GamePosition& game_state, const Move& move, unsigned int ply);
    // Calculate evaluation value based on Score, Liberties, Stones, stones in Atari, Euler
    double calc_eval(double score, int libs, int stones, int atari, double euler) const;
    // Evaluation function (heuristic function) for Go board (range: [-1, 1])
    double evaluate(const GamePosition& game_state);

    // Clear killer move list before every new search
    void clearKiller();
    // Insert killer move when there's alpha/beta cutoff
    void insertKiller(unsigned int ply, const Move& move);
    // Check if a move in a move list is a killer move when performing move ordering
    void checkKiller(unsigned int ply, std::vector<Move>& legalMoves);

    // Minimax value functions
    double max_value(const GamePosition& game_state, unsigned int depth, std::mt19937& rng);
    double min_value(const GamePosition& game_state, unsigned int depth, std::mt19937& rng);

    // Minimax value function with depth-limited minimax, NO ALPHA-BETA PRUNING
    double naive_minimax(const GamePosition& game_state, unsigned int depth, Stone player, std::mt19937& rng);
    // Minimax value function with depth-limited minimax, WITH ALPHA-BETA PRUNING
    double alpha_beta(const GamePosition& game_state, unsigned int depth, double alpha, double beta, Stone player, unsigned int ply, std::mt19937& rng);


    // Choose move for three difficulties
    Move choose_move_easy(const Game& game, std::mt19937& rng);
    Move choose_move_medium(const Game& game, std::mt19937& rng);
    Move choose_move_hard(const Game& game, unsigned int ply, std::mt19937& rng);
};
