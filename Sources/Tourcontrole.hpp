#include<iostream>
#include"Controleur.hpp"


class TourControle : public Controleur {
private:
    bool pisteLibre;
    std::map<std::string, bool> parkings; // P1, P2, P3...
    std::mutex pisteMutex;

public:
    TourControle(const std::string& id, int nbParkings);
    bool autoriserAtterrissage(Avion* avion);
    bool autoriserDecollage(Avion* avion);
    void libererPiste();
    void run() override;
};
