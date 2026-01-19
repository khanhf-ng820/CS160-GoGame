#include "AI_Utils/TranspositionTable.h"



TranspositionTable::TranspositionTable(size_t mb_size) {
	numEntries = (mb_size * 1024 * 1024) / sizeof(TTEntry);
	table.resize(numEntries);
}


TTEntry* TranspositionTable::getEntryPtr(uint64_t key) {
    return &table[key % numEntries];
}

bool TranspositionTable::probe(uint64_t key, Move& move, int8_t& depth, double& score, BoundFlag& bflag, uint8_t& age) {
	TTEntry* entry = getEntryPtr(key);
	if (entry->key == key) {
		move = entry->move;
		depth = entry->depth;
		score = entry->score;
		bflag = entry->bound;
		age = entry->age;
		return true;
	}
	return false;
}


void TranspositionTable::store(uint64_t key, Move move, int8_t depth, double score, BoundFlag bflag, uint8_t age) {
	TTEntry* entry = getEntryPtr(key);
	// -- Replacement strategy: Always replace if deeper or newer search
	if (entry->depth <= depth) {
		*entry = {key, move, depth, score, bflag, age};
	}
}
