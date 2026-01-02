#include "AI.h"






// BLACK is MAXimizing player
// WHITE is MINimizing player
// ***** Class for the Go AI *****
GoAI::GoAI(AIDifficulty d, std::mt19937* rng_ptr, std::mt19937_64* rng64_ptr) : diff(d), rng_ptr(rng_ptr), rng64_ptr(rng64_ptr) {
    for (std::vector<Move>& killerVector : killerMoves)
        killerVector.reserve(killerMoveStorage);
}
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

// ***** Action function that returns the MOST REASONABLE moves (for MEDIUM/HARD modes)
// Considerations for REASONABLE moves are written as comments in "AI.h"
// ===== Action function for MEDIUM MODE ONLY =====
std::vector<Move> GoAI::reasonableActionsMedium(const GamePosition& game_state, std::mt19937& rng) {
    size_t N = game_state.size();
    std::vector<Move> legalMoves; legalMoves.reserve(N * N + 1);

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

    // If there's no stone on the board, just play a random move
    if (legalMoves.size() == 1) {
        // Pick a random placing-stone move
        std::uniform_int_distribution<int> distrib(0, game_state.size() - 1);
        Move randomMove = {distrib(rng), distrib(rng), false};
        legalMoves[0] = randomMove;
    }

    // Shuffle all ALLOWED legal moves RANDOMLY
    std::shuffle(legalMoves.begin(), legalMoves.end(), rng);



    // === Limit to top k moves (by cheap heuristic function), where k = branchingFactorLimit
    size_t maxNumOfMoves = std::min(static_cast<size_t>(branchingFactorLimit), legalMoves.size());
    legalMoves.resize(maxNumOfMoves);


    // DEBUG
    // std::cout << legalMoves.size() << std::endl;

    return legalMoves;
}

// ===== Action function for HARD MODE ONLY =====
std::vector<Move> GoAI::reasonableActionsHard(const GamePosition& game_state, unsigned int ply, std::mt19937& rng) {
    size_t N = game_state.size();
    std::vector<Move> legalMoves; legalMoves.reserve(N * N + 1);

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
        // Pick a random placing-stone move
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

    // Evaluate all branches coming from a state
    std::vector<Branch> legalBranches(legalMoves.begin(), legalMoves.end());
    for (Branch& brch : legalBranches) {
        brch.eval = cheap_evaluate(game_state, brch.move, ply);
    }

    // === Move ordering using cheap evaluation function
    std::sort(legalBranches.begin(), legalBranches.end(), [](const Branch& a, const Branch& b) {
        return a.eval > b.eval;
    });
    for (int i = 0; i < legalMoves.size(); i++) {
        legalMoves[i] = legalBranches[i].move;
    }

    // === Limit to top k moves (by cheap evaluation function), where k = branchingFactorLimit
    size_t maxNumOfMoves = std::min(static_cast<size_t>(branchingFactorLimit), legalMoves.size());
    legalMoves.resize(maxNumOfMoves);

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

// Cheap evaluation function ONLY for MOVE ORDERING using killer heuristic
int GoAI::cheap_evaluate(const GamePosition& game_state, const Move& move, unsigned int ply) {
    // Check primary killer move
    // else if (killerMoves[ply].size() > 0 && move == killerMoves[ply][0]) {
    if (killerMoves[ply].size() > 0 && move == killerMoves[ply][0]) {
        return 900;
    }
    // Check secondary killer move
    else if (killerMoves[ply].size() > 1 && move == killerMoves[ply][1]) {
        return 800;
    }
    // Other quiet moves get 0 eval
    else {
        return 0;
    }
}

// Calculate evaluation value based on Score, Liberties, Stones, stones in Atari, Euler
double GoAI::calc_eval(double score, int libs, int stones, int atari, double euler) const {
    return scoreWeight * score
         + libsWeight * libs
         + stonesWeight * stones
         + atariWeight * atari
         + eulerWeight * euler;
}

// Evaluation function (heuristic function) for Go board (range: [-10000, 10000])
double GoAI::evaluate(const GamePosition& game_state) {
    // - If it's a terminal state
    if (terminal(game_state)) {
        double whiteScore = game_state.calcScore(Stone::WHITE), blackScore = game_state.calcScore(Stone::BLACK);
        if (blackScore > whiteScore) {
            return 10000.0;
        } else if (blackScore < whiteScore) {
            return -10000.0;
        } else
            return 0;
    }

    // - If it's a non-terminal state
    // Calculate each player's score
    double blackScore = game_state.calcScore(Stone::BLACK), whiteScore = game_state.calcScore(Stone::WHITE);
    // Count the number of liberties each player has
    int blackLibs, whiteLibs;
    game_state.countIndividualLiberty(blackLibs, whiteLibs);
    // Count how many stones each player has
    int blackStones, whiteStones;
    game_state.count(blackStones, whiteStones);
    // Count how many stones in atari each player has (to penalize)
    int blackAtari, whiteAtari;
    game_state.countAtari(blackAtari, whiteAtari);
    // Calculate the Euler number of the board with respect to each player
    double blackEuler, whiteEuler;
    game_state.calcEuler(blackEuler, whiteEuler);

    // Combined evaluation of each player, weight * value
    double blackEval = calc_eval(blackScore, blackLibs, blackStones, blackAtari, blackEuler);
    // DEBUG
    // std::cout << "blackScore: " << blackScore << std::endl;
    // std::cout << "blackLibs: " << blackLibs << std::endl;
    // std::cout << "blackStones: " << blackStones << std::endl;
    // std::cout << "blackEuler: " << blackEuler << std::endl;
    // std::cout << "blackAtari: " << blackAtari << std::endl;
    // std::cout << "-> blackEval: " << blackEval << std::endl;
    double whiteEval = calc_eval(whiteScore, whiteLibs, whiteStones, whiteAtari, whiteEuler);
    // DEBUG
    // std::cout << "whiteScore: " << whiteScore << std::endl;
    // std::cout << "whiteLibs: " << whiteLibs << std::endl;
    // std::cout << "whiteStones: " << whiteStones << std::endl;
    // std::cout << "whiteEuler: " << whiteEuler << std::endl;
    // std::cout << "whiteAtari: " << whiteAtari << std::endl;
    // std::cout << "=> whiteEval: " << whiteEval << std::endl;

    double finalEval = (blackEval - whiteEval) / (blackEval + whiteEval + 0.001);
    return std::min(std::max(finalEval, -10000.0), 10000.0);
}

// Clear killer move list before every new search
void GoAI::clearKiller() {
    std::fill(killerMoves.begin(), killerMoves.end(), std::vector<Move>());
}

// Insert killer move when there's alpha/beta cutoff
void GoAI::insertKiller(unsigned int ply, const Move& move) {
    // Safety check to prevent array out of bounds
    if (ply >= _MAX_PLIES_) return;
    // Shift and remove oldest killer move (if exists)
    for (int i = static_cast<int>(killerMoves[ply].size()) - 2; i >= 0; i--) {
        killerMoves[ply][i + 1] = killerMoves[ply][i];
    }
    if (killerMoves[ply].size() > 0) killerMoves[ply][0] = move;
    else killerMoves[ply].push_back(move);
}

// Check if a move in a move list is a killer move when performing move ordering
void GoAI::checkKiller(unsigned int ply, std::vector<Move>& legalMoves) {
    size_t movesCount = legalMoves.size();
    int priorityIdx = 0;
    for (int slot = 0; slot < killerMoves[ply].size(); slot++) {
        Move killerMove = killerMoves[ply][slot];
        for (int i = 0; i < movesCount; i++)
            if (legalMoves[i] == killerMove) {
                // legalMoves[i] is a killer move so move it up the list
                std::swap(legalMoves[i], legalMoves[priorityIdx]);
                priorityIdx++;
                std::cout << "Found killer move!\n";
                break;
        }
    }
}


// === Minimax value functions ===
// Max value function
double GoAI::max_value(const GamePosition& game_state, unsigned int depth, std::mt19937& rng) {
    if (depth == 0 || terminal(game_state)) {
        return evaluate(game_state);
    }
    double value = _NEGATIVE_INF_;

    std::vector<Move> allActions = reasonableActionsMedium(game_state, rng);
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

    std::vector<Move> allActions = reasonableActionsMedium(game_state, rng);
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
double GoAI::alpha_beta(const GamePosition& game_state, unsigned int depth, double alpha, double beta, Stone player, unsigned int ply, std::mt19937& rng) {
    if (depth == 0 || terminal(game_state)) {
        return evaluate(game_state);
    }

    if (player == Stone::BLACK) {
        // If it's maximizing player's turn
        double value = _NEGATIVE_INF_;

        std::vector<Move> allActions = reasonableActionsHard(game_state, ply, rng);

        for (const Move& mv : allActions) {
            value = std::max(value, alpha_beta(result(game_state, mv), depth - 1, alpha, beta, opposite(player), ply + 1, rng));
            if (value >= beta) {
                // Insert killer move at beta cutoff, if it's not a capture move
                if (!game_state.isCapture(mv)) insertKiller(depth, mv);
                break; // Beta cutoff
            }
            alpha = std::max(alpha, value);
        }
        return value;
    } else if (player == Stone::WHITE) {
        // If it's minimizing player's turn
        double value = _POSITIVE_INF_;

        // Move ordering is already done using cheap evaluation function
        std::vector<Move> allActions = reasonableActionsHard(game_state, ply, rng);
        
        for (const Move& mv : allActions) {
            value = std::min(value, alpha_beta(result(game_state, mv), depth - 1, alpha, beta, opposite(player), ply + 1, rng));
            if (value <= alpha) {
                // Insert killer move at alpha cutoff, if it's not a capture move
                if (!game_state.isCapture(mv)) insertKiller(depth, mv);
                break; // Alpha cutoff
            }
            beta = std::min(beta, value);
        }
        return value;
    } else { // Null player
        return 0;
    }
}


// Negamax value function, depth-limited, NO ALPHA-BETA PRUNING
double GoAI::naive_negamax(const GamePosition& game_state, unsigned int depth, int8_t color, std::mt19937& rng) {
    if (depth == 0 || terminal(game_state)) {
        return color * evaluate(game_state);
    }
    double value = _NEGATIVE_INF_;
    std::vector<Move> allActions = action(game_state, rng);

    for (const Move& mv : allActions) {
        GamePosition childState = result(game_state, mv);
        value = std::max(value, -naive_negamax(childState, depth - 1, -color, rng));
    }
    return value;
}

// Negamax value function, depth-limited, WITH ALPHA-BETA PRUNING
double GoAI::negamax_AB(const GamePosition& game_state, unsigned int depth, double alpha, double beta, int8_t color, std::mt19937& rng) {
    if (depth == 0 || terminal(game_state)) {
        return color * evaluate(game_state);
    }
    double value = _NEGATIVE_INF_;
    // Make sure to do Move Ordering
    std::vector<Move> allActions = action(game_state, rng);

    for (const Move& mv : allActions) {
        GamePosition childState = result(game_state, mv);
        value = std::max(value, -negamax_AB(childState, depth - 1, -beta, -alpha, -color, rng));
        alpha = std::max(alpha, value);
        if (alpha >= beta) {
            break;
        }
    }
    return value;
}





// CHOOSE MOVE DEPENDING ON DIFFICULTY
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

    // Choose move based on AI's difficulty
    switch (diff) {
    case AIDifficulty::EASY:
        return choose_move_easy(game, rng);
    case AIDifficulty::MEDIUM:
        return choose_move_medium(game, rng);
    case AIDifficulty::HARD:
        return choose_move_negamaxAB(game, hard_search_depth, rng);
        // Ply for killer heuristic
        // unsigned int ply = game.history_size() - 1;
        // return choose_move_hard(game, ply, rng);
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
    std::vector<Move> allActions = reasonableActionsMedium(game_state, rng);
    
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
    // DEBUG
    GamePosition futureState = game_state;
    futureState.play(bestMove);
    // - If it's a non-terminal state
    // Calculate each player's score
    double blackScore = futureState.calcScore(Stone::BLACK), whiteScore = futureState.calcScore(Stone::WHITE);
    // Count the number of liberties each player has
    int blackLibs, whiteLibs;
    futureState.countIndividualLiberty(blackLibs, whiteLibs);
    // Count how many stones each player has
    int blackStones, whiteStones;
    futureState.count(blackStones, whiteStones);
    // Count how many stones in atari each player has (to penalize)
    int blackAtari, whiteAtari;
    futureState.countAtari(blackAtari, whiteAtari);
    // Calculate the Euler number of the board with respect to each player
    double blackEuler, whiteEuler;
    futureState.calcEuler(blackEuler, whiteEuler);

    // Combined evaluation of each player, weight * value
    double blackEval = calc_eval(blackScore, blackLibs, blackStones, blackAtari, blackEuler);
    // DEBUG
    std::cout << "blackScore: " << blackScore << std::endl;
    std::cout << "blackLibs: " << blackLibs << std::endl;
    std::cout << "blackStones: " << blackStones << std::endl;
    std::cout << "blackEuler: " << blackEuler << std::endl;
    std::cout << "blackAtari: " << blackAtari << std::endl;
    std::cout << "-> blackEval: " << blackEval << std::endl;
    double whiteEval = calc_eval(whiteScore, whiteLibs, whiteStones, whiteAtari, whiteEuler);
    // DEBUG
    std::cout << "whiteScore: " << whiteScore << std::endl;
    std::cout << "whiteLibs: " << whiteLibs << std::endl;
    std::cout << "whiteStones: " << whiteStones << std::endl;
    std::cout << "whiteEuler: " << whiteEuler << std::endl;
    std::cout << "whiteAtari: " << whiteAtari << std::endl;
    std::cout << "=> whiteEval: " << whiteEval << std::endl;
    std::cout << "==> eval: " << (blackEval - whiteEval) / (blackEval + whiteEval + 0.001) << std::endl;

    return bestMove;
}

// ===== For HARD MODE =====
Move GoAI::choose_move_hard(const Game& game, unsigned int ply, std::mt19937& rng) {
    // Clear all killer moves saved
    clearKiller();

    GamePosition game_state = GamePosition(game);
    std::vector<Move> allActions = reasonableActionsHard(game_state, ply, rng);

    Move bestMove = _PASS_MOVE_;
    double bestValue = (game.side_to_move() == Stone::BLACK) ? _NEGATIVE_INF_ : _POSITIVE_INF_;
    for (const Move& mv : allActions) {
        double childValue = alpha_beta(result(game_state, mv), hard_search_depth - 1, _NEGATIVE_INF_, _POSITIVE_INF_, opposite(game.side_to_move()), ply + 1, rng);
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
}


// ===== Other 'choose move' functions, based on different AI techniques =====
// ---  ---
Move GoAI::choose_move_negamax(const Game& game, unsigned int depth, std::mt19937& rng) {
    GamePosition game_state = GamePosition(game);
    const Stone player = game_state.getPlayerToMove();
    std::vector<Move> allActions = action(game_state, rng);

    Move bestMove = _PASS_MOVE_;
    double bestValue = _NEGATIVE_INF_;
    assert(player != Stone::EMPTY);
    int8_t color = (player == Stone::BLACK) ? 1 : -1;
    for (const Move& mv : allActions) {
        GamePosition childState = result(game_state, mv);
        double childValue = -naive_negamax(childState, depth - 1, -color, rng);
        if (childValue > bestValue) {
            bestValue = childValue;
            bestMove = mv;
        }
    }

    return bestMove;
}

// Negamax with ALPHA-BETA PRUNING
Move GoAI::choose_move_negamaxAB(const Game& game, unsigned int depth, std::mt19937& rng) {
    GamePosition game_state = GamePosition(game);
    const Stone player = game_state.getPlayerToMove();
    std::vector<Move> allActions = action(game_state, rng);

    Move bestMove = _PASS_MOVE_;
    double bestValue = _NEGATIVE_INF_,
           alpha     = _NEGATIVE_INF_,
           beta      = _POSITIVE_INF_;
    assert(player != Stone::EMPTY);
    int8_t color = (player == Stone::BLACK) ? 1 : -1;
    for (const Move& mv : allActions) {
        GamePosition childState = result(game_state, mv);
        double childValue = -negamax_AB(childState, depth - 1, -beta, -alpha, -color, rng);
        if (childValue > bestValue) {
            bestValue = childValue;
            bestMove = mv;
        }
        alpha = std::max(alpha, childValue);
    }

    return bestMove;
}






// === OTHER NECESSARY METHODS ===
int GoAI::manhattan_dist(int r0, int c0, int r1, int c1) {
    return abs(r0-r1) + abs(c0-c1);
}
