#include "AI.h"


// ***** Class for a Game State *****
// Constructor
GamePosition::GamePosition(Board bd, Stone to_move, double komi) : Board(bd.size()), prevBoard(Board(bd.size())), to_move(to_move), komi(komi) {
    grid = bd.getGrid();
}
GamePosition::GamePosition(const Game& game) : Board(game.board().size()), prevBoard(Board(game.board().size())), to_move(game.side_to_move()), komi(game.komi()), consecutive_passes(game.get_consecutive_passes()), blacksCaptured(game.get_captured_stones(Stone::WHITE)), whitesCaptured(game.get_captured_stones(Stone::BLACK)) {
    grid = game.board().getGrid();
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






// BLACK is MAXimizing player
// WHITE is MINimizing player
// ***** Class for the Go AI *****
GoAI::GoAI(AIDifficulty d): diff(d) {}
AIDifficulty GoAI::getDiff() const { return diff; }

// *** FOR MINIMAX ALGORITHM ***
// Terminal function (when game ends)
bool GoAI::terminal(const GamePosition& game_state) {
    return game_state.gameEnded();
}

// Action function
// Returns RANDOMLY SHUFFLED vector of ALL POSSIBLE legal moves possible from that position, including pass
std::vector<Move> GoAI::action(const GamePosition& game_state, std::mt19937& rng) {
    size_t N = game_state.size();
    std::vector<Move> legalMoves; legalMoves.reserve(N * N + 1);

    // Consider all possible legal moves
    for (int r = 0; r < N; r++) {
        for (int c = 0; c < N; c++) {
            Move mv = {r,c,false};
            if (game_state.get(r, c) == Stone::EMPTY && game_state.legal(mv))
                legalMoves.push_back(mv);
        }
    }
    legalMoves.push_back( _PASS_MOVE_ ); // Pass

    std::shuffle(legalMoves.begin(), legalMoves.end(), rng);
    return legalMoves;
}

// Action function that returns the MOST REASONABLE moves (for MEDIUM/HARD modes)
// Considerations for REASONABLE moves are written as comments in "AI.h"
std::vector<Move> GoAI::reasonableActions(const GamePosition& game_state, std::mt19937& rng) {
    size_t N = game_state.size();
    std::vector<Move> legalMoves; legalMoves.reserve(N * N + 1);

    // ===== Action function for MEDIUM MODE ONLY =====
    if (diff == AIDifficulty::MEDIUM)   {
        // Consider all possible REASONABLE legal moves
        for (int r = 0; r < N; r++) {
            for (int c = 0; c < N; c++) {
                Move mv = {r,c,false};
                // Ignore illegal move
                if (!(game_state.get(r, c) == Stone::EMPTY && game_state.legal(mv)))
                    continue;
    
                // === Influence: Only consider empty points within Manhattan distance <= maxManDistMedium of any stone
                bool includeMove = false;
                for (int r0 = r-maxManDistMedium; r0 <= r+maxManDistMedium; r0++) {
                    if (includeMove) break;
                    for (int c0 = c-maxManDistMedium; c0 <= c+maxManDistMedium; c0++) {
                        // Check Manhattan distance of maxManDistMedium
                        if (game_state.in_bounds(r0, c0) && manhattan_dist(r, c, r0, c0) <= maxManDistMedium
                        && game_state.get(r0, c0) != Stone::EMPTY)
                            includeMove = true;
                    }
                }
    
                if (includeMove) legalMoves.push_back(mv);
            }
        }
    
        legalMoves.push_back( _PASS_MOVE_ ); // Pass is always included
    
        // Shuffle all ALLOWED legal moves RANDOMLY
        std::shuffle(legalMoves.begin(), legalMoves.end(), rng);
    
    
    
        // === Limit to top k moves (by cheap heuristic function)
        // std::sort(legalMoves.begin(), legalMoves.end(), [this, &game_state](const Move& a, const Move& b) {
        //     double evalA = evaluate(result(game_state, a)), evalB = evaluate(result(game_state, b));
        //     if (game_state.getPlayerToMove() == Stone::BLACK) {
        //         return evalA > evalB;
        //     } else if (game_state.getPlayerToMove() == Stone::WHITE) {
        //         return evalA < evalB;
        //     } else { return true; } // Null player
        // });
        // size_t maxNumOfMoves = std::min(static_cast<size_t>(branchingFactorLimit), legalMoves.size());
        // legalMoves.resize(maxNumOfMoves);
    } else if (diff == AIDifficulty::HARD) {
        // ===== Action function for HARD MODE ONLY =====
        // == Consider all possible REASONABLE legal moves
        // Take the most recent move (most recently placed stone / pass)
        Move mostRecentMove = _PASS_MOVE_;
        bool foundMostRecentMv = false;
        for (int r = 0; r < N; r++) {
            if (foundMostRecentMv) break;
            for (int c = 0; c < N; c++) {
                if (game_state.get(r, c) != Stone::EMPTY && game_state.getPrevBoard().get(r, c) == Stone::EMPTY) {
                    mostRecentMove = {r,c,false};
                    foundMostRecentMv = true;
                }
            }
        }

        // If the most recent move is a PASS move
        if (mostRecentMove.is_pass) {
            // Pick a random move
            std::uniform_int_distribution<int> distrib(0, game_state.size() - 1);
            mostRecentMove = {distrib(rng), distrib(rng), false};
        }
        // Move: placing a stone with Chebyshev distance <= maxCheDistHard from mostRecentMove
        for (int r_offset = -maxCheDistHard; r_offset <= maxCheDistHard; r_offset++) {
            for (int c_offset = -maxCheDistHard; c_offset <= maxCheDistHard; c_offset++) {
                int r = mostRecentMove.r, c = mostRecentMove.c;
                int r0 = r + r_offset, c0 = c + c_offset;
                Move mv = {r0, c0, false};
                // Check Chebyshev distance of maxCheDistHard and check legal move
                if (game_state.in_bounds(r0, c0)
                // && manhattan_dist(r, c, r0, c0) <= maxCheDistHard
                && game_state.get(r0, c0) == Stone::EMPTY && game_state.legal(mv))
                    legalMoves.push_back(mv);
            }
        }

        legalMoves.push_back( _PASS_MOVE_ ); // Pass is always included
    
        // Shuffle all ALLOWED legal moves RANDOMLY
        std::shuffle(legalMoves.begin(), legalMoves.end(), rng);
    
    
    
        // === Limit to top k moves (by evaluation function)
        // std::sort(legalMoves.begin(), legalMoves.end(), [this, &game_state](const Move& a, const Move& b) {
        //     double evalA = evaluate(result(game_state, a)), evalB = evaluate(result(game_state, b));
        //     if (game_state.getPlayerToMove() == Stone::BLACK) {
        //         return evalA > evalB;
        //     } else if (game_state.getPlayerToMove() == Stone::WHITE) {
        //         return evalA < evalB;
        //     } else { return true; } // Null player
        // });
        // size_t maxNumOfMoves = std::min(static_cast<size_t>(branchingFactorLimit), legalMoves.size());
        // legalMoves.resize(maxNumOfMoves);
    }

    // DEBUG
    // std::cout << legalMoves.size() << std::endl;

    return legalMoves;
}

// Result function (new game state from old game state + a legal move)
GamePosition GoAI::result(const GamePosition& game_state, const Move& move) {
    GamePosition new_game_state = game_state;
    new_game_state.play(move);
    return new_game_state;
}

// Evaluation function (heuristic function) for Go board (range: [-1, 1])
double GoAI::evaluate(const GamePosition& game_state) {
    // If it's a terminal state
    if (terminal(game_state)) {
        double whiteScore = game_state.calcScore(Stone::WHITE), blackScore = game_state.calcScore(Stone::BLACK);
        if (blackScore > whiteScore) {
            return 1;
        } else if (blackScore < whiteScore) {
            return -1;
        } else
            return 0;
    }

    // If it's a non-terminal state
    // Using the same scoring function instead of hand-crafted heuristic
    double whiteScore = game_state.calcScore(Stone::WHITE), blackScore = game_state.calcScore(Stone::BLACK);
    return (blackScore - whiteScore) / (blackScore + whiteScore + 0.001);
}

// Minimax value functions
// Max value function
double GoAI::max_value(const GamePosition& game_state, unsigned int depth, std::mt19937& rng) {
    if (depth == 0 || terminal(game_state)) {
        return evaluate(game_state);
    }
    double value = _NEGATIVE_INF_;

    std::vector<Move> allActions = reasonableActions(game_state, rng);
    for (const Move& mv : allActions) {
        value = std::max(value, min_value(result(game_state, mv), depth - 1, rng));
    }
    return value;
}

// Min value function
double GoAI::min_value(const GamePosition& game_state, unsigned int depth, std::mt19937& rng) {
    if (depth == 0 || terminal(game_state)) {
        return evaluate(game_state);
    }
    double value = _POSITIVE_INF_;

    std::vector<Move> allActions = reasonableActions(game_state, rng);
    for (const Move& mv : allActions) {
        value = std::min(value, max_value(result(game_state, mv), depth - 1, rng));
    }
    return value;
}

// Minimax value function with depth-limited minimax, NO ALPHA-BETA PRUNING
double GoAI::naive_minimax(const GamePosition& game_state, unsigned int depth, Stone player, std::mt19937& rng) {
    switch (player) {
    case Stone::BLACK:
        // If it's maximizing player's turn
        return max_value(game_state, depth, rng);
    case Stone::WHITE:
        // If it's minimizing player's turn
        return min_value(game_state, depth, rng);
    default:
        // No player
        return 0;
    }
}

// Minimax value function with depth-limited minimax, with ALPHA-BETA PRUNING
double GoAI::alpha_beta(const GamePosition& game_state, unsigned int depth, double alpha, double beta, Stone player, std::mt19937& rng) {
    if (depth == 0 || terminal(game_state)) {
        return evaluate(game_state);
    }

    if (player == Stone::BLACK) {
        // If it's maximizing player's turn
        double value = _NEGATIVE_INF_;

        std::vector<Move> allActions = reasonableActions(game_state, rng);
        for (const Move& mv : allActions) {
            value = std::max(value, alpha_beta(result(game_state, mv), depth - 1, alpha, beta, opposite(player), rng));
            if (value >= beta) 
                break; // Beta cutoff
            alpha = std::max(alpha, value);
        }
        return value;
    } else if (player == Stone::WHITE) {
        // If it's minimizing player's turn
        double value = _POSITIVE_INF_;

        std::vector<Move> allActions = reasonableActions(game_state, rng);
        for (const Move& mv : allActions) {
            value = std::min(value, alpha_beta(result(game_state, mv), depth - 1, alpha, beta, opposite(player), rng));
            if (value <= alpha)
                break; // Alpha cutoff
            beta = std::min(beta, value);
        }
        return value;
    } else { // Null player
        return 0;
    }
}



// Choose move depending on difficulty
Move GoAI::choose_move(const Game& game, std::mt19937& rng){
    const int N = game.size();
    // std::vector<Move> cand; cand.reserve(N*N+1);

    // for (int r = 0; r < N; r++)
    //     for (int c = 0; c < N; c++)
    //         if (game.board().get(r,c)==Stone::EMPTY)
    //             cand.push_back( {r,c,false} );

    // if (cand.empty()) return _PASS_MOVE_; // pass

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
        return choose_move_medium(game, rng);
    case AIDifficulty::HARD:
        return choose_move_hard(game, rng);
    };
}

// ===== For EASY MODE =====
Move GoAI::choose_move_easy(const Game& game, std::mt19937& rng) {
    GamePosition game_state = GamePosition(game);
    std::vector<Move> legalMoves = action(game_state, rng);

    // Pick a random move
    std::uniform_int_distribution<int> distrib(0, static_cast<int>(legalMoves.size()) - 1);
    return legalMoves[distrib(rng)];
}

// ===== For MEDIUM MODE =====
Move GoAI::choose_move_medium(const Game& game, std::mt19937& rng) {
    GamePosition game_state = GamePosition(game);
    std::vector<Move> allActions = reasonableActions(game_state, rng);
    
    Move bestMove = _PASS_MOVE_;
    double bestValue = (game.side_to_move() == Stone::BLACK) ? _NEGATIVE_INF_ : _POSITIVE_INF_;
    for (const Move& mv : allActions) {
        double childValue = naive_minimax(result(game_state, mv), medium_search_depth - 1, opposite(game.side_to_move()), rng);
        // DEBUG
        // std::cout <<"Move: "<< mv.r<<' '<<mv.c<<' '<<mv.is_pass << " childValue: " << childValue << " bestValue: " << bestValue << '\n';
        if ((game.side_to_move() == Stone::BLACK && childValue > bestValue)
        || (game.side_to_move() == Stone::WHITE && childValue < bestValue)) {
            bestMove = mv;
            bestValue = childValue;
        }
    }
    // DEBUG
    std::cout <<"Move: "<< bestMove.r<<' '<<bestMove.c<<' '<<bestMove.is_pass << " bestValue: " << bestValue << '\n';

    return bestMove;

    return _PASS_MOVE_; // ***** PLACEHOLDER, WILL DELETE LATER *****
}

// ===== For HARD MODE =====
Move GoAI::choose_move_hard(const Game& game, std::mt19937& rng) {
    GamePosition game_state = GamePosition(game);
    std::vector<Move> allActions = reasonableActions(game_state, rng);

    Move bestMove = _PASS_MOVE_;
    double bestValue = (game.side_to_move() == Stone::BLACK) ? _NEGATIVE_INF_ : _POSITIVE_INF_;
    for (const Move& mv : allActions) {
        double childValue = alpha_beta(result(game_state, mv), hard_search_depth - 1, _NEGATIVE_INF_, _POSITIVE_INF_, opposite(game.side_to_move()), rng);
        // DEBUG
        // std::cout <<"Move: "<< mv.r<<' '<<mv.c<<' '<<mv.is_pass << " childValue: " << childValue << " bestValue: " << bestValue << '\n';
        if ((game.side_to_move() == Stone::BLACK && childValue > bestValue)
        || (game.side_to_move() == Stone::WHITE && childValue < bestValue)) {
            bestMove = mv;
            bestValue = childValue;
        }
    }
    // DEBUG
    std::cout <<"Move: "<< bestMove.r<<' '<<bestMove.c<<' '<<bestMove.is_pass << " bestValue: " << bestValue << '\n';

    return bestMove;

    return _PASS_MOVE_; // ***** PLACEHOLDER, WILL DELETE LATER *****
}





// === OTHER NECESSARY METHODS ===
int GoAI::manhattan_dist(int r0, int c0, int r1, int c1) {
    return abs(r0-r1) + abs(c0-c1);
}
