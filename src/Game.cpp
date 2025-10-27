#include "Board.h"
#include "Game.h"
#include <iostream>
#include <vector>



namespace Game {
	std::vector<std::vector<int>> history(1, Board::board); // Board playing history
	int currentHistoryIdx = 0;
	int player = Board::BLACK;

	void changePlayer() {
		player = Board::WHITE + Board::BLACK - player;
	}

	// Returns bool whether placing piece is successful
	bool placePiece(std::vector<int>& board, int piecePos) {
		if (piecePos == Board::INVALID_INTERSECTION) return false;

		// Game mechanics
		// Resolve (WIP)
		board[piecePos] = player;

		changePlayer();
		return true;
	}

	void moveHistoryUp() {
		history.erase(history.begin() + currentHistoryIdx + 1, history.end());
		history.push_back(Board::board);
		currentHistoryIdx++;
	}

	void pass(std::vector<int>& board) {
		changePlayer();

		history.erase(history.begin() + currentHistoryIdx + 1, history.end());
		history.push_back(board);
		currentHistoryIdx++;
	}

	void undo() {
		if (currentHistoryIdx > 0) {
			currentHistoryIdx--;
			Board::board = history[currentHistoryIdx];

			changePlayer();
		}
	}

	void redo() {
		if (currentHistoryIdx < history.size() - 1) {
			currentHistoryIdx++;
			Board::board = history[currentHistoryIdx];

			changePlayer();
		}
	}
}
