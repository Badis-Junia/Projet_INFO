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

    sf::Sprite backgroundSprite(backgroundImage), avionSprite(avionTexture), aeroportSprite(aeroportTexture), aeroport2Sprite(aeroportTexture),aeroport3Sprite(aeroportTexture),aeroport4Sprite(aeroportTexture),aeroport5Sprite(aeroportTexture),aeroport6Sprite(aeroportTexture),aeroport7Sprite(aeroportTexture),aeroport8Sprite(aeroportTexture),aeroport9Sprite(aeroportTexture),aeroport10Sprite(aeroportTexture) ;

std::vector<Aeroport> aeroports = {
    {"Oregon",     100, 200},
    {"Texas",      400, 600},
    {"Ohio",       925, 380},
    {"Montana",    390, 180},
    {"Colorado",   410, 425},
    {"NewYork",   1100, 230},
    {"Tennessy",   900, 500},
    {"Floride",   1025, 750},
    {"Californie", 175, 500},
    {"Iowa",       700, 350}
};
    Aeroport aeroport2 = aeroports[0];

    Avion avionTest("10", "AirTest");
    avionSprite.setScale(sf::Vector2f(0.8, 0.8));
    avionTest.setVitesse(100.0); 
    avionTest.setPosition(aeroport2.getPositionX(), aeroport2.getPositionY(), 0);
    std::vector<sf::Sprite> aeroportsprite = {aeroportSprite, aeroport2Sprite, aeroport3Sprite, aeroport4Sprite, aeroport5Sprite, aeroport6Sprite, aeroport7Sprite, aeroport8Sprite, aeroport9Sprite, aeroport10Sprite};

    for(long unsigned int i = 0;i<aeroportsprite.size(); i++) {
        aeroportsprite[i].setScale(sf::Vector2f(0.12, 0.12));
        aeroportsprite[i].setPosition(sf::Vector2f(aeroports[i].getPositionX(), aeroports[i].getPositionY()));
    }



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
            avionTest.setDestination(aeroports[8].getPositionX(), aeroports[8].getPositionY());
            avionSprite.setRotation(avionTest.inclinaison());
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

        std::vector<sf::Sprite> tableausprite = {backgroundSprite, avionSprite};        

        for(long unsigned int i = 0; i < tableausprite.size(); i++) {
            app.draw(tableausprite[i]);
        }

        for (auto& aeroportSprite : aeroportsprite) {
            app.draw(aeroportSprite);
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
