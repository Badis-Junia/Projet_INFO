#include<iostream>
#include"Controleur.hpp"

class ControleurApproche : public Controleur {
private:
    Controleur* twr; // lien vers la tour de contrôle

public:
    ControleurApproche(const std::string& id, Controleur* twr);
    void assignerTrajectoire(Avion* avion);
    void gererUrgence(Avion* avion);
    void demanderAutorisationAtterrissage(Avion* avion);
    void run() override;
};
