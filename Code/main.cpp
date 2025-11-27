#include <SFML/Graphics.hpp>
#include <iostream>
#include "Sources.hpp"
// test
int main() {
    try {
        Simulation simulation;
        simulation.executer();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Erreur: " << e.what() << std::endl;
        return -1;
    }
}
