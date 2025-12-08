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

// Action function that returns the MOST REASONABLE moves
// Considerations for REASONABLE moves are written as comments in "AI.h"
std::vector<Move> GoAI::reasonableActions(const GamePosition& game_state, std::mt19937 rng) {
    size_t N = game_state.size();
    std::vector<Move> legalMoves; legalMoves.reserve(N * N + 1);

    // Consider all possible legal moves
    for (int r = 0; r < N; r++) {
        for (int c = 0; c < N; c++) {
            Move mv = {r,c,false};
            // Ignore illegal move
            if (!(game_state.get(r, c) == Stone::EMPTY && game_state.legal(mv)))
                continue;

            // === Influence: Only consider empty points within Manhattan distance <= 3 of any stone
            bool includeMove = false;
            for (int r0 = r-3; r0 <= r+3; r0++) {
                if (includeMove) break;
                for (int c0 = c-3; c0 <= c+3; c0++) {
                    // Check Manhattan distance of 3
                    if (game_state.in_bounds(r0, c0) && manhattan_dist(r, c, r0, c0) <= 3
                    && game_state.get(r0, c0) != Stone::EMPTY)
                        includeMove = true;
                }
            }
            if (includeMove) legalMoves.push_back(mv);
        }
    }
    legalMoves.push_back( _PASS_MOVE_ ); // Pass is always included
    // DEBUG
    // std::cout << legalMoves.size() << std::endl;

    std::shuffle(legalMoves.begin(), legalMoves.end(), rng);
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
    return (blackScore - whiteScore) / (blackScore + whiteScore);
    // if (blackScore > whiteScore) {
    //     return 1;
    // } else if (blackScore < whiteScore) {
    //     return -1;
    // } else
    //     return 0;

    return 0; // ***** PLACEHOLDER *****
}

// Minimax value functions
// Max value function
double GoAI::max_value(const GamePosition& game_state, int depth, std::mt19937& rng) {
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
double GoAI::min_value(const GamePosition& game_state, int depth, std::mt19937& rng) {
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
double GoAI::depth_minimax(const GamePosition& game_state, int depth, Stone player, std::mt19937& rng) {
    if (player == Stone::BLACK) {
        // If it's maximizing player's turn
        return max_value(game_state, depth, rng);
    } else if (player == Stone::WHITE) {
        // If it's minimizing player's turn
        return min_value(game_state, depth, rng);
    } else { // Null player
        return 0;
    }
}

// Minimax value function with depth-limited minimax, with ALPHA-BETA PRUNING
double GoAI::alpha_beta(const GamePosition& game_state, int depth, double alpha, double beta, Stone player, std::mt19937& rng) {
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
        return choose_move_easy(game, rng);
    };
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
    legalMoves.push_back( _PASS_MOVE_ ); // Pass

    // Pick a random move
    std::uniform_int_distribution<int> distrib(0, static_cast<int>(legalMoves.size()) - 1);
    return legalMoves[distrib(rng)];
}

// For MEDIUM MODE
Move GoAI::choose_move_medium(const Game& game, std::mt19937& rng) {
    GamePosition game_state = GamePosition(game);
    std::vector<Move> allActions = reasonableActions(game_state, rng);
    
    Move bestMove = _PASS_MOVE_;
    double bestValue = (game.side_to_move() == Stone::BLACK) ? _NEGATIVE_INF_ : _POSITIVE_INF_;
    for (const Move& mv : allActions) {
        double childValue = depth_minimax(result(game_state, mv), medium_search_depth - 1, opposite(game.side_to_move()), rng);
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

// For HARD MODE
Move GoAI::choose_move_hard(const Game& game, std::mt19937& rng) {
    return _PASS_MOVE_; // ***** PLACEHOLDER, WILL DELETE LATER *****
}





// === PRIVATE METHODS ===
int GoAI::manhattan_dist(int r0, int c0, int r1, int c1) {
    return abs(r0-r1) + abs(c0-c1);
}
