// #include <cstdlib>
// #include <iostream>
#include <SFML/Graphics.hpp>

#include "Sources.hpp"



#ifdef _MSC_VER
// #pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif

const std::string path_image(PATH_IMG);

int main() {

	const sf::Vector2u WINDOW_SIZE(800, 600);
    sf::RenderWindow app(sf::VideoMode({WINDOW_SIZE.x, WINDOW_SIZE.y}, 32), "Projet_INFO");

    app.setFramerateLimit(60); 

    return 0;
}


