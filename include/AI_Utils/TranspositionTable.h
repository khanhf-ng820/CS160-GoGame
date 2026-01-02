#pragma once
#include <vector>
#include <cstdint>
#include "Game.h"



enum class BoundFlag : unsigned char { EXACT, UPPER, LOWER };

struct TTEntry {
	uint64_t  key;   // Zobrist hash
	Move      move;
	int8_t    depth;
	double    score;
	BoundFlag bound;
	uint8_t   age;
};



// Transposition table class
class TranspositionTable {
public:
	// Constructor based on size of table in MB
	explicit TranspositionTable(size_t mb_size);
	// Get pointer to entry in vector
	TTEntry* getEntryPtr(uint64_t key);
	// Probe
	bool probe(uint64_t key, Move& move, int8_t& depth, double& score, BoundFlag& bflag, uint8_t& age);
	// Store new entry
	// -- Replacement strategy: Always replace if deeper or newer search
	void store(uint64_t key, Move move, int8_t depth, double score, BoundFlag bflag, uint8_t age);

private:
	std::vector<TTEntry> table;
	size_t numEntries;
};
