#include<string>
#include<thread>
#include <mutex>
#include <vector>
#include <map>
#include <fstream>
class Agent {
protected:
    int id;
    std::thread thread_;
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
public:
    void afficherAPP(const std::vector<Avion*>& avions);
    void afficherTWR(const std::vector<Avion*>& avions);
};





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





class Monde {
private:
    std::vector<std::unique_ptr<Avion>> avions;
    std::unique_ptr<CentreControleRegional> ccr;
    std::unique_ptr<ControleurApproche> app;
    std::unique_ptr<TourControle> twr;

public:
    Monde();
    void initialiser();
    void demarrerSimulation();
    void arreterSimulation();
};





class Journal {
private:
    std::mutex mtx;
    std::ofstream fichier;

public:
    Journal(const std::string& nomFichier);
    void log(const std::string& message);
};

