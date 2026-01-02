#pragma once
#include <iostream>
#include <random>
#include <cstdint>
#include "AI_Utils/GamePosition.h"



class Rand64BitstringGenerator {
public:
	explicit Rand64BitstringGenerator(std::mt19937_64* rng64_ptr);
	// Function to generate a random 64-bit bitstring as uint64_t
	uint64_t generateRandom();

private:
	std::mt19937_64* rng64_ptr;
	std::uniform_int_distribution<uint64_t> distrib;
};



class ZobristHasher {
public:
	// Constructor
	explicit ZobristHasher(std::mt19937_64* rng64_ptr);
	// Calculate Zobrist hash from a game state (GamePosition)
	uint64_t calcHash(const GamePosition& game_state) const;

	// 64bit bitstring generator
	Rand64BitstringGenerator bitstrGen;

private:
	// All generated bitstrings
	uint64_t blackToMove_bitstr;
	std::vector<std::vector<uint64_t>> piecePosition_bitstr;
};
