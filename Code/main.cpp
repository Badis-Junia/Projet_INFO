#include <SFML/Graphics.hpp>
#include <iostream>
#include "Sources.hpp"

int main() {
    try {
        Simulation simulation;
        simulation.executer();
        std::cout << "simulation lancé " << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Erreur: " << e.what() << std::endl;
        return -1;
    }
}
