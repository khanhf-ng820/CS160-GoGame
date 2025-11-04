#include "Board.h"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>
#include <vector>



namespace Board {
	// Consts
	// enum class State {
	// 	INVALID_INTERSECTION,
	// 	EMPTY,
	// 	BLACK,
	// 	WHITE
	// };

	// enum class Liberty {
	// 	EMPTY_INT,
	// 	NO_LIBERTY,
	// 	HAS_LIBERTY
	// };

	const int INVALID_INTERSECTION = -1;



	bool enabled = true;
	int boardWidth = 19, boardHeight = 19;
	float boardSize = 500.f;
	float squareLength = boardSize / (boardWidth - 1);
	float pieceRadius = squareLength / 2 - 2.f;

	sf::FloatRect boardRect({25.f, 25.f}, {550.f, 550.f});


	// 1D vector of 2D playing board
	std::vector<State> board(boardWidth * boardHeight, State::EMPTY);
	std::vector<State> hasLiberty(boardWidth * boardHeight, State::EMPTY);
	std::vector<bool> visited(boardWidth * boardHeight, false);



	// Change the board size and reset new game
	void changeBoardSize(int bWidth, int bHeight) {
		boardWidth = bWidth;
		boardHeight = bHeight;
		squareLength = boardSize / (boardWidth - 1);

		board = std::vector<State>(boardWidth * boardHeight, State::EMPTY);
	}

	int piecePosFromMousePos(sf::Vector2f mousePos, Board::State& player) {
		sf::Vector2f boardMousePos = mousePos - sf::Vector2f({50.f, 50.f});
		sf::Vector2f intersectionPos = boardMousePos / squareLength;
		int intersectionX = round(intersectionPos.x);
		int intersectionY = round(intersectionPos.y);

		if (intersectionX >= 0 && intersectionX < boardWidth && intersectionY >= 0
		&& intersectionY < boardHeight && board[ix(intersectionX, intersectionY)] == State::EMPTY) {
			// Get 1D index of the intersection position on the board
			int idx = ix(intersectionX, intersectionY);

			return idx; // Returns the 1D index
		}
		return INVALID_INTERSECTION; // Not on any intersection of the board
	}

	// Resolve captures when trying to place a piece
	int resolveTurn(int& player, int piecePos) {
		// WIP
		std::vector<bool> visited(boardWidth * boardHeight, false);
	}

	void displayBoard(sf::RenderWindow& window) {
		// Display board
		for (int i = 0; i < boardWidth; i++) {
			for (int j = 0; j < boardHeight; j++) {
				sf::Vector2f position = {50.f + squareLength * i, 50.f + squareLength * j};

				// Draw the board
				if (i < boardWidth - 1 && j < boardHeight - 1) {
					sf::RectangleShape square({squareLength, squareLength});
					square.setFillColor(sf::Color::Transparent);
					square.setOutlineThickness(1.f);
					square.setOutlineColor(sf::Color::Black);

					square.setPosition(position);

					window.draw(square);
				}

				// Draw the pieces
				sf::CircleShape piece(pieceRadius);
				piece.setFillColor(sf::Color::Transparent);
				piece.setOrigin({pieceRadius, pieceRadius});
				piece.setPosition(position);

				if (board[j * boardWidth + i] == State::WHITE) {
					piece.setFillColor(sf::Color::White);
				} else if (board[j * boardWidth + i] == State::BLACK) {
					piece.setFillColor(sf::Color::Black);
				}

				window.draw(piece);
			}
		}
	}

	bool mouseOnBoard(sf::Vector2f mousePos) {
		return boardRect.contains(mousePos);
	}

	void enable() {
		enabled = true;
	}

	void disable() {
		enabled = false;
	}

	int ix(int x, int y) {
		return y * boardWidth + x;
	}
}
