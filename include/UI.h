#pragma once
#ifndef UI_H
#define UI_H

#include <SFML/Graphics.hpp>
#include <iostream>

namespace UI {
	class RectButton {
	public:
		bool enabled;
		sf::Vector2f topLeft, rectSize;
		std::string text;
		unsigned int textSize;
		sf::Color textFillColor, rectColor, hoverColor;
		sf::RectangleShape rectEntity;
		sf::FloatRect rect, textBoundingBox;

		RectButton(sf::Vector2f rectSize, sf::Vector2f topLeft, std::string s, 
			unsigned int tSize, sf::Color textColor, sf::Color rectClr, sf::Font& font);
		RectButton(sf::Vector2f rectSize, sf::Vector2f topLeft, std::string s, unsigned int tSize, 
			sf::Color textColor, sf::Color rectClr, sf::Color hoverClr, sf::Font& font);
		void setString(std::string s);
		bool mouseOnButton(sf::Vector2f mousePos);
		void hoverChangeColor(sf::Vector2f mousePos);
		void draw(sf::RenderWindow& window);
		void enable();
		void disable();
	private:
		sf::Text textEntity;
	};
}

#endif
