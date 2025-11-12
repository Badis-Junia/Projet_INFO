#include<iostream>
#include<thread>
#include "Agent.hpp"

class Avion : public Agent {
private:
    std::string compagnie;
    double positionX, positionY, altitude;
    double vitesse;
    std::string etat;
    bool urgence;

public:
    Avion(const std::string& id, const std::string& compagnie);
    ~Avion();
    void run() override;
    void majposition();
    void atterisage();
    void decollage();
    void Urgence(bool etatUrgence);
    bool estEnUrgence() const;

};