#pragma once
#ifndef BOARD_H
#define BOARD_H

#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>
#include <vector>

namespace Board {
	extern const int INVALID_INTERSECTION, EMPTY, BLACK, WHITE;
	
	extern bool enabled;
	extern int boardWidth, boardHeight;
	extern float boardSize, squareLength, pieceRadius;

	extern sf::FloatRect boardRect;

	extern std::vector<int> board;

	void changeBoardSize(int bWidth, int bHeight);
	int piecePosFromMousePos(sf::Vector2f mousePos, int& player);
	void displayBoard(sf::RenderWindow& window);
	bool mouseOnBoard(sf::Vector2f mousePos);
	void enable();
	void disable();
	int ix(int x, int y);
}

#endif
