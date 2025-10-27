#include "Board.h"
#include "UI.h"
#include "Game.h"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#define MAIN_MENU 100
#define IN_GAME 101
#define END_GAME 102
#define PVP 200
#define PVE 201


int main()
{
	std::cout << "CS160 Go Game Final Project" << std::endl;

	int appState = IN_GAME;
	int gameMode = PVP;


	// Screen size
	unsigned int screenWidth = 800u;
	unsigned int screenHeight = 600u;

	// Create window
	auto window = sf::RenderWindow(sf::VideoMode({screenWidth, screenHeight}), "CS160 Go Game Project");
	window.setFramerateLimit(60);

	// Load font
	sf::Font font;
	if (!font.openFromFile("../assets/fonts/RobotoMono-VariableFont_wght.ttf"))
		std::cerr << "Error loading font!" << std::endl;


	// Load UI textboxes
	UI::RectButton playerToMoveTextbox(sf::Vector2f(150.f, 50.f), sf::Vector2f(600.f, 50.f), 
		"Black to move", 18, sf::Color::Black, sf::Color::White, font);

	// // Create text SFML entity
	// sf::Text textEntity(font);
	// textEntity.setString("Black to move");
	// textEntity.setCharacterSize(18);
	// textEntity.setFillColor(sf::Color::Black);
	// // Set position of text entity
	// sf::FloatRect textBoundingBox = textEntity.getGlobalBounds();
	// textEntity.setOrigin(textBoundingBox.size / 2.f);
	// textEntity.setPosition(playerToMoveTextbox.topLeft + playerToMoveTextbox.rectSize / 2.f);


	// Load UI buttons
	UI::RectButton passButton(sf::Vector2f(150.f, 50.f), sf::Vector2f(600.f, 110.f), 
		"Pass", 18, sf::Color::Black, sf::Color::White, sf::Color::Green, font);

	// // Create text SFML entity
	// sf::Text passtextEntity(font);
	// passtextEntity.setString("Pass");
	// passtextEntity.setCharacterSize(18);
	// passtextEntity.setFillColor(sf::Color::Black);
	// // Set position of text entity
	// sf::FloatRect textBoundingBox = passtextEntity.getGlobalBounds();
	// passtextEntity.setOrigin(textBoundingBox.size / 2.f);
	// passtextEntity.setPosition(passButton.topLeft + passButton.rectSize / 2.f);

	UI::RectButton undoButton(sf::Vector2f(70.f, 50.f), sf::Vector2f(600.f, 170.f), 
		"Undo", 18, sf::Color::Black, sf::Color::White, sf::Color::Green, font);

	UI::RectButton redoButton(sf::Vector2f(70.f, 50.f), sf::Vector2f(680.f, 170.f), 
		"Redo", 18, sf::Color::Black, sf::Color::White, sf::Color::Green, font);


	auto enableAllUI = [&]() {
		Board::enable();
		passButton.enable();
		undoButton.enable();
		redoButton.enable();
	};
	auto disableAllUI = [&]() {
		Board::disable();
		passButton.disable();
		undoButton.disable();
		redoButton.disable();
	};


	while (window.isOpen())
	{
		while (const std::optional event = window.pollEvent())
		{
			if (event->is<sf::Event::Closed>())
			{
				window.close();
			}
		}

		window.clear(sf::Color(255, 193, 140));



		// Get mouse position
		sf::Vector2f localMousePosition = sf::Vector2f(sf::Mouse::getPosition(window));


		// If in game
		if (appState == IN_GAME) {
			// ----- Mouse left down
			if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
				// Clicked Pass button
				if (passButton.mouseOnButton(localMousePosition) && passButton.enabled) {
					// Change player
					Game::pass(Board::board);
				}

				// Clicked Undo button
				else if (undoButton.mouseOnButton(localMousePosition) && undoButton.enabled) {
					// Undo a move
					Game::undo();
				}

				else if (redoButton.mouseOnButton(localMousePosition) && redoButton.enabled) {
					// Redo a move
					Game::redo();
				}

				// Player plays on board
				else if (Board::mouseOnBoard(localMousePosition) && Board::enabled) {
					// Place a piece
					int piecePos = Board::piecePosFromMousePos(localMousePosition, Game::player);

					// Game mechanics
					// Resolve (WIP)
					bool placePieceSuccessful = Game::placePiece(Board::board, piecePos);

					if (placePieceSuccessful) {
						Game::moveHistoryUp();
					}
				}


				// Disable everything
				disableAllUI();
			} else {
				enableAllUI();
			}


			// ----- Mouse hover
			// UI textbox hover
			// if (playerToMoveTextbox.mouseOnButton(localMousePosition)) {
			// 	playerToMoveTextbox.rectEntity.setFillColor(sf::Color::Green);
			// } else {
			// 	playerToMoveTextbox.rectEntity.setFillColor(sf::Color::White);
			// }
			playerToMoveTextbox.setString((Game::player == Board::BLACK) ? "Black to move" : "White to move");

			// UI button hover
			passButton.hoverChangeColor(localMousePosition);
			undoButton.hoverChangeColor(localMousePosition);
			redoButton.hoverChangeColor(localMousePosition);


		
			// Display board
			Board::displayBoard(window);


			// Draw textboxes
			playerToMoveTextbox.draw(window);

			// Draw buttons
			passButton.draw(window);
			undoButton.draw(window);
			redoButton.draw(window);
		}



		// Draw mouse
		sf::CircleShape mouseCirc(5.f);
		mouseCirc.setFillColor(sf::Color::Transparent);
		mouseCirc.setOrigin({5.f, 5.f});
		mouseCirc.setPosition(localMousePosition);
		mouseCirc.setOutlineThickness(2.f);
		mouseCirc.setOutlineColor({255, 0, 0});

		window.draw(mouseCirc);



		window.display();
	}
}
