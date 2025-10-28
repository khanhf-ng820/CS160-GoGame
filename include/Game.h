#pragma once
#ifndef GAME_H
#define GAME_H

#include <iostream>
#include <vector>


namespace Game {
	extern std::vector<std::vector<int>> history;
	extern int currentHistoryIdx, player;

	void changePlayer();
	bool playPiece(std::vector<int>& board, int piecePos);
	void moveHistoryUp();
	void pass(std::vector<int>& board);
	void undo();
	void redo();
};


#endif
