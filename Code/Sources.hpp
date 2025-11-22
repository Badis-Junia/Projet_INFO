#include<string>
#include<thread>
#include <mutex>
#include <vector>
#include <map>
#include <fstream>
#include <memory>


class Agent {
protected:
    int id;
    std::thread monthread;
    bool actif;

public:
    Agent(const int & id);
    virtual void run() = 0;
    void start();
    void stop();

    std::string getId() const;
};





class Avion : public Agent {
private:
    std::string compagnie;
    double positionX, positionY, positionZ;
    double vitesse;
    std::string etat;
    bool urgence;
    double carburant;
    double consommation;

public:
    Avion(const std::string& id, const std::string& compagnie);
    ~Avion();
    void run() override;
    void majPosition();
    void atterrissage();
    void decollage();
    void declarerUrgence(bool etatUrgence);
    bool estEnUrgence() const;
    double getCarburant() const;
    void consommerCarburant();

};






class Controleur : public Agent {
protected:
    std::mutex mutex;
    std::vector<Avion> avionsSousControle;

public:
    Controleur(const std::string& id);
    virtual void recevoirAvion(Avion avion);
    virtual void libererAvion(Avion* avion);
    virtual void run() override = 0;
};






class ControleurApproche : public Controleur {
private:
    Controleur* tour; 
    std::vector<Avion*> fileAttente;

public:
    ControleurApproche(const std::string& id, Controleur* tour);
    void assignerTrajectoire(Avion* avion);
    void gererUrgence(Avion* avion);
    void demanderAutorisationAtterrissage(Avion* avion);
    void run() override;
};






class CentreControleRegional : public Controleur {
private:
    std::vector<ControleurApproche*> approchesLiees;

public:
    CentreControleRegional(const std::string& id);
    void ajouterApproche(ControleurApproche* app);
    void transfererVol(Avion* avion, ControleurApproche* app);
    void run() override;
};






class InterfaceGraphique {
private:
    std::mutex mutexAffichage;

public:
    void afficherAPP(const std::vector<Avion*>& avions);
    void afficherTWR(const std::vector<Avion*>& avions);
    void afficherCCR(const std::vector<Avion*>& avions);
};






class TourControle : public Controleur {
private:
    bool pisteLibre;
    std::map<std::string, bool> parkings;
    std::mutex mutexPiste;
    std::vector<Avion*> fileAttenteDecollage;

public:
    TourControle(const std::string& id, int nbParkings);
    bool autoriserAtterrissage(Avion* avion);
    bool autoriserDecollage(Avion* avion);
    void libererPiste();
    void attribuerParking(Avion* avion);
    void run() override;
};






class Monde {
private:
    std::vector<std::unique_ptr<Avion>> avions;
    std::unique_ptr<CentreControleRegional> ccr;
    std::unique_ptr<ControleurApproche> app;
    std::unique_ptr<TourControle> twr;
    InterfaceGraphique interface;

public:
    Monde();
    void initialiser();
    void demarrerSimulation();
    void arreterSimulation();
    void ajouterAvion(std::unique_ptr<Avion> avion);
};






class Journal {
private:
    std::mutex mutex;
    std::ofstream fichier;

public:
    Journal(const std::string& nomFichier);
    void log(const std::string& message);
    ~Journal();
};
