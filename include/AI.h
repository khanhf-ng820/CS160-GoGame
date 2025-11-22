#pragma once
#include <random>
#include "Game.h"

enum class AIDifficulty { EASY, MEDIUM, HARD };

class GoAI {
public:
    explicit GoAI(AIDifficulty d=AIDifficulty::EASY);

    // Heuristic function for Go board
    double heuristic(const Game& game);

    // For minimax algorithm
    double max_value(Board bd);
    double min_value(Board bd);

    // Choose move depending on difficulty
    Move choose_move(const Game& game, std::mt19937& rng);

    // For three difficulties
    Move choose_move_easy(const Game& game, std::mt19937& rng);
    Move choose_move_medium(const Game& game, std::mt19937& rng);
    Move choose_move_hard(const Game& game, std::mt19937& rng);

private:
    AIDifficulty diff;
    int medium_search_depth = 2;
};
