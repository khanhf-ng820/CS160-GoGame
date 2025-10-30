#pragma once
#ifndef BOARD_H
#define BOARD_H

#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>
#include <vector>

namespace Board {
	// Consts
	enum class State {
		EMPTY,
		BLACK,
		WHITE
	};

	enum class Liberty {
		EMPTY_INT,
		NO_LIBERTY,
		HAS_LIBERTY
	};


	extern const int INVALID_INTERSECTION;
	
	extern bool enabled;
	extern int boardWidth, boardHeight;
	extern float boardSize, squareLength, pieceRadius;

	extern sf::FloatRect boardRect;

	extern std::vector<State> board, hasLiberty;

	void changeBoardSize(int bWidth, int bHeight);
	int piecePosFromMousePos(sf::Vector2f mousePos, Board::State& player);
	int resolveTurn(int& player, int piecePos);
	void displayBoard(sf::RenderWindow& window);
	bool mouseOnBoard(sf::Vector2f mousePos);
	void enable();
	void disable();
	int ix(int x, int y);
}

#endif
