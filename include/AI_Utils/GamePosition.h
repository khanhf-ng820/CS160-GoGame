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



// ***** Class for a Game State (Game position) *****
class GamePosition : public Board {
public:
    // Constructor
    explicit GamePosition(Board bd, Stone to_move, const double& komi);
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
    const double& komi;
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
