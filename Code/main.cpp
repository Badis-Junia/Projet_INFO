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

    sf::Sprite backgroundSprite(backgroundImage), avionSprite(avionTexture), aeroportSprite(aeroportTexture), aeroport2Sprite(aeroportTexture),aeroport3Sprite(aeroportTexture),aeroport4Sprite(aeroportTexture),aeroport5Sprite(aeroportTexture),aeroport6Sprite(aeroportTexture),aeroport7Sprite(aeroportTexture),aeroport8Sprite(aeroportTexture),aeroport9Sprite(aeroportTexture),aeroport10Sprite(aeroportTexture) ;
    Aeroport aeroport("Oregon", 100, 200);
    Aeroport aeroport2("Texas", 400, 600);        
    Aeroport aeroport3("Ohio", 925, 380);        
    Aeroport aeroport4("Montana", 390, 180);        
    Aeroport aeroport5("Colorado", 410, 425);        
    Aeroport aeroport6("NewYork", 1100, 230);        
    Aeroport aeroport7("Tennessy", 900, 500);        
    Aeroport aeroport8("Floride", 1025, 750);        
    Aeroport aeroport9("Californie", 175, 500);        
    Aeroport aeroport10("Iowa", 700, 350);        



    Avion avionTest("10", "AirTest");

    avionTest.setPosition(aeroport.getPositionX(), aeroport.getPositionY(), 0); 
    avionTest.setVitesse(100.0); 

    
    avionSprite.setScale(sf::Vector2f(0.8, 0.8));

    aeroportSprite.setScale(sf::Vector2f(0.12, 0.12));
    aeroport2Sprite.setScale(sf::Vector2f(0.12, 0.12));
    aeroport3Sprite.setScale(sf::Vector2f(0.12, 0.12));
    aeroport4Sprite.setScale(sf::Vector2f(0.12, 0.12));
    aeroport5Sprite.setScale(sf::Vector2f(0.12, 0.12));
    aeroport6Sprite.setScale(sf::Vector2f(0.12, 0.12));
    aeroport7Sprite.setScale(sf::Vector2f(0.12, 0.12));
    aeroport8Sprite.setScale(sf::Vector2f(0.12, 0.12));
    aeroport9Sprite.setScale(sf::Vector2f(0.12, 0.12));
    aeroport10Sprite.setScale(sf::Vector2f(0.12, 0.12));

    
    aeroportSprite.setPosition(sf::Vector2f(aeroport.getPositionX(), aeroport.getPositionY()));
    aeroport2Sprite.setPosition(sf::Vector2f(aeroport2.getPositionX(), aeroport2.getPositionY() - 20));
    aeroport3Sprite.setPosition(sf::Vector2f(aeroport3.getPositionX(), aeroport3.getPositionY() - 20));
    aeroport4Sprite.setPosition(sf::Vector2f(aeroport4.getPositionX(), aeroport4.getPositionY() - 20));
    aeroport5Sprite.setPosition(sf::Vector2f(aeroport5.getPositionX(), aeroport5.getPositionY() - 20));
    aeroport6Sprite.setPosition(sf::Vector2f(aeroport6.getPositionX(), aeroport6.getPositionY() - 20));
    aeroport7Sprite.setPosition(sf::Vector2f(aeroport7.getPositionX(), aeroport7.getPositionY() - 20));
    aeroport8Sprite.setPosition(sf::Vector2f(aeroport8.getPositionX(), aeroport8.getPositionY() - 20));
    aeroport9Sprite.setPosition(sf::Vector2f(aeroport9.getPositionX(), aeroport9.getPositionY() - 20));
    aeroport10Sprite.setPosition(sf::Vector2f(aeroport10.getPositionX(), aeroport10.getPositionY() - 20));

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

        std::vector<sf::Sprite> tableausprite = {backgroundSprite, aeroportSprite, aeroport2Sprite, aeroport3Sprite, aeroport4Sprite, aeroport5Sprite, aeroport6Sprite, aeroport7Sprite, aeroport8Sprite, aeroport9Sprite, aeroport10Sprite, avionSprite};        

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
