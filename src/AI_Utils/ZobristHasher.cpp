#include "AI_Utils/ZobristHasher.h"



Rand64BitstringGenerator::Rand64BitstringGenerator(std::mt19937_64* rng64_ptr) :
	rng64_ptr( rng64_ptr ), distrib( std::numeric_limits<uint64_t>::min(), std::numeric_limits<uint64_t>::max() ) {}

uint64_t Rand64BitstringGenerator::generateRandom() {
	return distrib(*rng64_ptr);
}



ZobristHasher::ZobristHasher(std::mt19937_64* rng64_ptr) :
	bitstrGen( rng64_ptr ), piecePosition_bitstr( 19 * 19, std::vector<uint64_t>(2) )
{
	blackToMove_bitstr = bitstrGen.generateRandom();
	for (int i = 0; i < 19 * 19; i++) {
		piecePosition_bitstr[i][0] = bitstrGen.generateRandom(); // Stone::BLACK
		piecePosition_bitstr[i][1] = bitstrGen.generateRandom(); // Stone::WHITE
	}
}

// Calculate Zobrist hash from a game state (GamePosition)
uint64_t ZobristHasher::calcHash(const GamePosition& game_state) const {
	uint64_t hashStr = 0;
	if (game_state.getPlayerToMove() == Stone::BLACK) {
		hashStr ^= blackToMove_bitstr;
	}
	// Loop over all board intersections
	const int boardSize = game_state.size();
	for (int i = 0; i < boardSize; i++) {
		for (int j = 0; j < boardSize; j++) {
			const Stone pieceColor = game_state.get(i, j);
			const int index_1D = game_state.idx1D(i, j);
			switch (pieceColor) {
			case Stone::BLACK:
				hashStr ^= piecePosition_bitstr[index_1D][0];
				break;
			case Stone::WHITE:
				hashStr ^= piecePosition_bitstr[index_1D][1];
				break;
			default:
				break;
			}
		}
	}

	return hashStr;
}
