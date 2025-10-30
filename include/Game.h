#pragma once
#ifndef GAME_H
#define GAME_H

#include "Board.h"
#include <iostream>
#include <vector>


namespace Game {
	extern std::vector<std::vector<Board::State>> history;
	extern int currentHistoryIdx;
	extern Board::State player;

	void changePlayer();
	bool playPiece(std::vector<Board::State>& board, int piecePos);
	void moveHistoryUp();
	void pass(std::vector<Board::State>& board);
	void undo();
	void redo();
};


#endif
