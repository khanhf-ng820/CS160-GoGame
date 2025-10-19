#include "Board.h"
#include <SFML/Graphics.hpp>

int main()
{
	sayHelloBoard();
    auto window = sf::RenderWindow(sf::VideoMode({1920u, 1080u}), "CS160 Go Game Project");
    window.setFramerateLimit(60);

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
        }

        window.clear();
        window.display();
    }
}
