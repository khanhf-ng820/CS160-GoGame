#include "Board.h"
#include "Game.h"
#include <iostream>
#include <vector>



namespace Game {
	std::vector<std::vector<Board::State>> history(1, Board::board); // Board playing history
	int currentHistoryIdx = 0;
	Board::State player = Board::State::BLACK;

	void changePlayer() {
		if (player == Board::State::BLACK) {
			player = Board::State::WHITE;
		} else if (player == Board::State::WHITE) {
			player = Board::State::BLACK;
		}
	}

	// Returns bool whether placing piece is successful
	bool playPiece(std::vector<Board::State>& board, int piecePos) {
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

	void pass(std::vector<Board::State>& board) {
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
