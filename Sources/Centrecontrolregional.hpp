#include<iostream>
#include"Controleur.hpp"

class CentreControleRegional : public Controleur {
private:
    std::vector<ControleurApproche*> approchesLiees;

public:
    CentreControleRegional(const std::string& id);
    void ajouterApproche(ControleurApproche* app);
    void transfererVol(Avion* avion, ControleurApproche* app);
    void run() override;
};
