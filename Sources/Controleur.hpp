#include<iostream>
#include<mutex>
#include "Agent.hpp"


class Controleur : public Agent {
protected:
    std::mutex mtx;
    std::vector<Avion> avionsSousControle;

public:
    Controleur(const std::string& id);
    virtual void recevoirAvion(Avion avion);
    virtual void libererAvion(Avion* avion);
    virtual void run() override = 0;
};