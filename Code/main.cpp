#include <SFML/Graphics.hpp>
#include <iostream>
#include "Sources.hpp"
#include <vector>
#include <math.h>






#ifdef _MSC_VER

#endif




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
    Aeroport aeroport("Oregon", 100, 200);
    Aeroport aeroport2("Texas", 400, 600);        

    Avion avionTest("10", "AirTest");

    avionTest.setPosition(aeroport.getPositionX(), aeroport.getPositionY(), 0); 
    avionTest.setVitesse(100.0); 

    
    avionSprite.setScale(sf::Vector2f(0.8, 0.8));

    aeroportSprite.setScale(sf::Vector2f(0.12, 0.12));
    aeroport2Sprite.setScale(sf::Vector2f(0.12, 0.12));

    
    aeroportSprite.setPosition(sf::Vector2f(aeroport.getPositionX(), aeroport.getPositionY()));
    aeroport2Sprite.setPosition(sf::Vector2f(aeroport2.getPositionX(), aeroport2.getPositionY() - 20));

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
            if (volDemarre && avionTest.getEtat() == "en vol" && 
                sqrt(pow(avionTest.getPositionX() - aeroport2.getPositionX(), 2) + 
                     pow(avionTest.getPositionY() - aeroport2.getPositionY(), 2)) < 50) {
                avionTest.atterrissage();
            }
            volDemarre = true;
            std::cout << "vol vers " << aeroport2.getId() << std::endl;
        }

        avionSprite.setPosition(sf::Vector2f(static_cast<float>(avionTest.getPositionX()), static_cast<float>(avionTest.getPositionY())));
        
        static int counter = 0;
        if (counter++ % 60 == 0) {
            std::cout << "Position avion: (" << avionTest.getPositionX() << ", " << avionTest.getPositionY() << ", " << avionTest.getPositionZ() << ")" << " - carburant: " << avionTest.getCarburant() << " - état: " << avionTest.getEtat() << std::endl;
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
