#include <SFML/Graphics.hpp>
#include <iostream>
#include "Sources.hpp"
#include <vector>
#include <math.h>

const std::string path_image("../Pictures/");

int lasimulation() {
    Monde monde;
    monde.initialiser();
    monde.demarrerSimulation();

    const sf::Vector2u WINDOW_SIZE(1300, 805);
    sf::RenderWindow app(sf::VideoMode({WINDOW_SIZE.x, WINDOW_SIZE.y}, 32), "Projet_INFO");
    app.setFramerateLimit(60);

    sf::Texture backgroundImage, avionTexture, aeroportTexture;

    if (!backgroundImage.loadFromFile(path_image + "background.png") || !avionTexture.loadFromFile(path_image + "avion.png") || !aeroportTexture.loadFromFile(path_image + "aeroport.png")) {
        std::cerr << "Erreur pendant le chargement des images" << std::endl;
        return -1;
    }

    sf::Sprite backgroundSprite(backgroundImage), avionSprite(avionTexture), aeroportSprite(aeroportTexture), aeroport2Sprite(aeroportTexture);
    CentreControleRegional aeroport("10");
    CentreControleRegional aeroport2("11");        

    Avion avionTest("10", "AirTest");

    aeroport2.setPositionX(400);
    aeroport2.setPositionY(600);

    avionTest.setPosition(100, 200, 0); 
    avionTest.setVitesse(100.0); 

    
    avionSprite.setScale(sf::Vector2f(1, 1));

    aeroportSprite.setScale(sf::Vector2f(0.2, 0.2));
    aeroport2Sprite.setScale(sf::Vector2f(0.2, 0.2));

    
    aeroportSprite.setPosition(sf::Vector2f(100, 200));
    aeroport2Sprite.setPosition(sf::Vector2f(400, 560));


    bool volDemarre = false;
    
    avionTest.start();
    int go = 0;
    while (app.isOpen()) {
        go++;
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
        if(go == 1) {
            avionTest.decollage();
            avionTest.setDestination(aeroport2.getPositionX(), aeroport2.getPositionY());
            volDemarre = true;
            std::cout << "vol vers aéroport 2" << std::endl;
        }


        
        avionSprite.setPosition(sf::Vector2f(
            static_cast<float>(avionTest.getPositionX()), 
            static_cast<float>(avionTest.getPositionY())
        ));

        
        static int counter = 0;
        if (counter++ % 60 == 0) {
            std::cout << "Position avion: (" << avionTest.getPositionX() << ", " 
                      << avionTest.getPositionY() << ") - état: " << avionTest.getEtat() << std::endl;
        }

        app.clear();

        std::vector<sf::Sprite> tableausprite = {backgroundSprite, aeroportSprite, aeroport2Sprite, avionSprite};        

        for(long unsigned int i = 0; i < tableausprite.size(); i++) {
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
