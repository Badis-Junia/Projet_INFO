#include <SFML/Graphics.hpp>
#include <iostream>
#include "Sources.hpp"

const std::string path_image("../Pictures/");

int main() {
    Monde monde;
    monde.initialiser();
    monde.demarrerSimulation();

    const sf::Vector2u WINDOW_SIZE(1300, 805);
    sf::RenderWindow app(sf::VideoMode({WINDOW_SIZE.x, WINDOW_SIZE.y}, 32), "Projet_INFO");
    app.setFramerateLimit(60);

    sf::Texture backgroundImage;
    sf::Texture avionTexture;
    sf::Texture tourControleTexture;


    if (!backgroundImage.loadFromFile(path_image + "background.png") ||
        !avionTexture.loadFromFile(path_image + "avion.png") ||
        !tourControleTexture.loadFromFile(path_image + "tourcontrole.png")) {
        std::cerr << "Erreur pendant le chargement des images" << std::endl;
        return -1;
    }

    sf::Sprite backgroundSprite(backgroundImage);
    sf::Sprite avionSprite(avionTexture);
    sf::Sprite tourControleSprite(tourControleTexture);
    

    avionSprite.setScale(sf::Vector2f(0.2f, 0.2f));
    

    tourControleSprite.setScale(sf::Vector2f(0.3f, 0.3f));
    tourControleSprite.setPosition(sf::Vector2f(1000.f, 500.f)); 

    Avion avionTest("10", "AirTest");
    avionTest.start();

    while (app.isOpen()) {
        while (std::optional event = app.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                app.close();
            }
            if (auto* keyEvent = event->getIf<sf::Event::KeyPressed>()) {
                if (keyEvent->code == sf::Keyboard::Key::Enter) {
                    app.close();
                }
            }
        }

        
        avionSprite.setPosition(sf::Vector2f(
            static_cast<float>(avionTest.getPositionX() / 10.f), 
            static_cast<float>(avionTest.getPositionY() / 10.f)
        ));

        app.clear();
        app.draw(backgroundSprite);
        app.draw(tourControleSprite); 
        app.draw(avionSprite);
        app.display();
    }

    avionTest.stop();
    monde.arreterSimulation();

    return 0;
}
