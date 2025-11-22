// #include <cstdlib>
// #include <iostream>
#include <SFML/Graphics.hpp>
#include <iostream>
#include "Sources.hpp"



#ifdef _MSC_VER

#endif

const std::string path_image("../Pictures/");

int main() {

	const sf::Vector2u WINDOW_SIZE(1298, 805);
    sf::RenderWindow app(sf::VideoMode({WINDOW_SIZE.x, WINDOW_SIZE.y}, 32), "Projet_INFO");
    app.setFramerateLimit(60); 


    sf::Texture backgroundImage(path_image + "background.png");
    sf::Sprite backgroundSprite(backgroundImage);

	if (!backgroundImage.loadFromFile(path_image + "background.png"))
	{
        std::cerr << "Erreur pendant le chargement des images" << std::endl;
        return 0;
	}

	backgroundSprite.setTexture(backgroundImage);


	while (app.isOpen()) 
	{
		while (const std::optional event = app.pollEvent())
		{
			if ((event->is<sf::Event::KeyPressed>() && event->getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::Escape) ||
				event->is<sf::Event::Closed>())
			{
				app.close();
			}
		}

		app.clear();
		app.draw(backgroundSprite);
		app.display();
	}

    return 0;
}


