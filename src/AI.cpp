#include "AI.h"
#include <vector>
#include <algorithm>

GoAI::GoAI(AIDifficulty d): diff(d) {}

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
        return choose_move_medium(game, rng);
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
    
}
