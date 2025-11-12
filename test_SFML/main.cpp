#include <cstdlib>
#include <iostream>
#include <SFML/Graphics.hpp>

#include "Car.hpp"

using namespace std;
using namespace sf;

#ifdef _MSC_VER
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif

const std::string path_image(PATH_IMG);

int main()
{
	// Fenêtre
	const Vector2u WINDOW_SIZE(800, 600);

	RenderWindow app(VideoMode({WINDOW_SIZE.x, WINDOW_SIZE.y}, 32), "My Camera");

	// Frames Per Second (FPS)
	app.setFramerateLimit(60); // limite la fenêtre à 60 images par seconde

	// Fond d'écran
	Texture backgroundImage(path_image + "background.png"), carImage(path_image + "car.png"), runnerImage;
	Sprite backgroundSprite(backgroundImage), carSprite(carImage), runnerSprite(runnerImage);

	if (!backgroundImage.loadFromFile(path_image + "background.png") ||
		!carImage.loadFromFile(path_image + "car.png"))
	{
		cerr << "Erreur pendant le chargement des images" << endl;
		return EXIT_FAILURE; // On ferme le programme
	}

	backgroundSprite.setTexture(backgroundImage);
	carSprite.setTexture(carImage);
	carSprite.setPosition(sf::Vector2f(20, 34));
	carSprite.setScale(sf::Vector2f(0.5, 0.5));

	sf::IntRect rect({0, 0}, {102, 115});

	constexpr int shift_x = 50;
	constexpr int shift_y = 10;

	runnerSprite.setTexture(runnerImage);
	runnerSprite.setTextureRect(rect);
	runnerSprite.setScale(sf::Vector2f(0.3f, 0.3f));

	Car car(790, 1215, 0, 0);

	Vector2f center(car.getX(), car.getY());

	Vector2f halfSize(WINDOW_SIZE.x / 2.f, WINDOW_SIZE.y / 2.f);

	View view(center, halfSize);

	app.setView(view);

	while (app.isOpen()) // Boucle principale
	{
		while (const std::optional event = app.pollEvent())
		{
			if ((event->is<sf::Event::KeyPressed>() && event->getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::Escape) ||
				event->is<sf::Event::Closed>())
			{
				app.close();
			}
		}
		car.move();
		view.setCenter({car.getX(), car.getY()});

		app.setView(view);

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
		{
			car.turnLeft();
			carSprite.setRotation(sf::degrees(car.getAngle()));
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
		{
			car.turnRight();
			carSprite.setRotation(sf::degrees(car.getAngle()));
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
		{
			car.speedUp();
		}
		else
		{
			car.speedDown();
		}

		// Affichages
		app.clear();
		app.draw(backgroundSprite);

		carSprite.setPosition({car.getX(), car.getY()});

		app.draw(carSprite);

		app.display();
	}

	return EXIT_SUCCESS;
}
