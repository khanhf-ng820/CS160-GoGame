#include "UI.h"
#include <SFML/Graphics.hpp>
#include <iostream>



UI::RectButton::RectButton(sf::Vector2f rectSizeV2f, sf::Vector2f topLeftV2f, std::string s, 
	unsigned int tSize, sf::Color textColor, sf::Color rectClr, sf::Font& font) : textEntity(font) {
	// Initialize
	enabled = true;
	text = s;
	textSize = tSize;
	textFillColor = textColor;
	rectColor = rectClr;
	hoverColor = rectColor;
	topLeft = topLeftV2f;
	rectSize = rectSizeV2f;

	// Create FloatRect
	rect = sf::FloatRect(topLeft, rectSize);

	// Create rect entity
	rectEntity = sf::RectangleShape(rectSize);
	rectEntity.setFillColor(sf::Color::White);

	// Set position of rect entity
	rectEntity.setPosition(topLeft);


	// Create text SFML entity
	// textEntity = sf::Text(font);
	textEntity.setString(text);
	textEntity.setCharacterSize(textSize);
	textEntity.setFillColor(textFillColor);

	// Set position of text entity
	textBoundingBox = textEntity.getGlobalBounds();
	textEntity.setOrigin(textBoundingBox.size / 2.f);
	textEntity.setPosition(topLeft + rectSize / 2.f);
}

UI::RectButton::RectButton(sf::Vector2f rectSizeV2f, sf::Vector2f topLeftV2f, std::string s, 
	unsigned int tSize, sf::Color textColor, sf::Color rectClr, sf::Color hoverClr, sf::Font& font) : textEntity(font) {
	// Initialize
	enabled = true;
	text = s;
	textSize = tSize;
	textFillColor = textColor;
	rectColor = rectClr;
	hoverColor = hoverClr;
	topLeft = topLeftV2f;
	rectSize = rectSizeV2f;

	// Create FloatRect
	rect = sf::FloatRect(topLeft, rectSize);

	// Create rect entity
	rectEntity = sf::RectangleShape(rectSize);
	rectEntity.setFillColor(sf::Color::White);

	// Set position of rect entity
	rectEntity.setPosition(topLeft);


	// Create text SFML entity
	// textEntity = sf::Text(font);
	textEntity.setString(text);
	textEntity.setCharacterSize(textSize);
	textEntity.setFillColor(textFillColor);

	// Set position of text entity
	textBoundingBox = textEntity.getGlobalBounds();
	textEntity.setOrigin(textBoundingBox.size / 2.f);
	textEntity.setPosition(topLeft + rectSize / 2.f);
}


void UI::RectButton::setString(std::string s) {
	text = s;
	textEntity.setString(s);
}

bool UI::RectButton::mouseOnButton(sf::Vector2f mousePos) {
	return rect.contains(mousePos);
}

void UI::RectButton::hoverChangeColor(sf::Vector2f mousePos) {
	if (mouseOnButton(mousePos)) {
		rectEntity.setFillColor(hoverColor);
	} else {
		rectEntity.setFillColor(rectColor);
	}
}

void UI::RectButton::draw(sf::RenderWindow& window) {
	window.draw(rectEntity);
	window.draw(textEntity);
}

void UI::RectButton::enable() {
	enabled = true;
}

void UI::RectButton::disable() {
	enabled = false;
}

