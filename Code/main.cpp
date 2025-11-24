#include <SFML/Graphics.hpp>
#include <iostream>
#include "Sources.hpp"
#include <vector>

const std::string path_image("../Pictures/");

int lasimulation() {
    Monde monde;
    monde.initialiser();
    monde.demarrerSimulation();

    const sf::Vector2u WINDOW_SIZE(1300, 805);
    sf::RenderWindow app(sf::VideoMode({WINDOW_SIZE.x, WINDOW_SIZE.y}, 32), "Projet_INFO");
    app.setFramerateLimit(60);


    sf::Texture backgroundImage, avionTexture, tourControleTexture, aeroportTexture;

    if (!backgroundImage.loadFromFile(path_image + "background.png") ||
        !avionTexture.loadFromFile(path_image + "avion.png") ||
        !tourControleTexture.loadFromFile(path_image + "tourcontrole.png") || 
        !aeroportTexture.loadFromFile(path_image + "aeroport.png")) {
            std::cerr << "Erreur pendant le chargement des images" << std::endl;
            return -1;
    }

    sf::Sprite backgroundSprite(backgroundImage), avionSprite(avionTexture), tourControleSprite(tourControleTexture), aeroportSprite(aeroportTexture), aeroport2Sprite(aeroportTexture);
    
    
    Avion avionTest("10", "AirTest");
    CentreControleRegional aeroport1("10");
    CentreControleRegional aeroport2("11");

    avionSprite.setScale(sf::Vector2f(1, 1));

    tourControleSprite.setScale(sf::Vector2f(0.3f, 0.3f));
    tourControleSprite.setPosition(sf::Vector2f(1000.f, 500.f)); 


    aeroportSprite.setScale(sf::Vector2f(0.2, 0.2));


    aeroport2Sprite.setScale(sf::Vector2f(0.2, 0.2));




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

        aeroportSprite.setPosition(sf::Vector2f(
            static_cast<float>(1000 / 10.f),
            static_cast<float>(2000 / 10.f)

        ));

        aeroport2Sprite.setPosition(sf::Vector2f(
            static_cast<float>(aeroport2.getPositionX() / 10.f),
            static_cast<float>(aeroport2.getPositionY() / 10.f)

        ));
        app.clear();

        std::vector<sf::Sprite> tableausprite = {backgroundSprite, tourControleSprite, aeroportSprite, aeroport2Sprite, avionSprite};        

        for(long unsigned int i = 0;i<tableausprite.size();i++) {
            app.draw(tableausprite[i]);
        }


        app.display();
    }

    avionTest.stop();
    monde.arreterSimulation();

    return 0;
}

int main() {
    lasimulation();
    return 0;
}
