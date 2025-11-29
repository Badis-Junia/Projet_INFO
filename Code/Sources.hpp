#include <string>
#include <thread>
#include <mutex>
#include <vector>
#include <map>
#include <fstream>
#include <memory>
#include <SFML/Graphics.hpp>

class Temps {
private:
    double facteurTemps;
    const double facteur_acceleration = 0.2;
    const double facteur_max = 5.0;
    const double facteur_min = 0.1;
    int minute;
    int heure;
    std::mutex mutex;

public:
    Temps() : facteurTemps(1) {}
    
    double getFacteurTemps();
    int getMinute();
    int getHeure();
    void accelererTemps();
    void ralentirTemps();
    void resetTemps();
    friend std::ostream& operator<<(std::ostream& flux, const Temps & temps);
};





class Agent {
protected:
    int id;
    std::thread monthread;
    bool actif;

public:
    Agent(const int & id);
    virtual ~Agent() = default;
    
    Agent(const Agent&) = delete;
    Agent& operator=(const Agent&) = delete;
    
    virtual void run() = 0;
    void start();
    void stop();
    std::string getId() const;
};

class Aeroport {
private:
    std::string id;
    double positionX;
    double positionY;
public:
    Aeroport(const std::string & id, double positionX, double positionY) : id(id), positionX(positionX), positionY(positionY) {};
    double getPositionX();
    double getPositionY();
    std::string getId();
    void setPosition(double unepositionX, double unepositionY);
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
    bool enDeplacement;  
    double destinationX, destinationY;  
    double altitudeCible;
    double penteApproche;
    bool enApprocheFinale;
    double distanceAtterissage = 300.0;
    double vitesseNormal = 400.0;
    double vitesseApproche = 200.0;
    double vitesseFinale = 100.0;
    double vitesseAtterrissage = 50.0;
    bool estgare;
    bool bienausol;
    bool estgaré;
    sf::Angle angle;
    Temps& tempsRef;

public:
    Avion(const std::string& id, const std::string& compagnie, Aeroport & aeroport, Temps& temps);
    ~Avion();
    bool volDemarre = false;
    void setPosition(double x, double y, double z);
    void setVitesse(double nouvelleVitesse);    
      
    Avion(const Avion&) = delete;
    Avion& operator=(const Avion&) = delete;
    
    void run() override;
    void majPosition();
    void atterrissage();
    void decollage();
    void declarerUrgence(bool etatUrgence);
    bool estEnUrgence() const;
    double getCarburant() const;
    void consommerCarburant(std::string etat);
    double getPositionX() const;
    double getPositionY() const;
    double getPositionZ() const { return positionZ; }
    std::string getEtat() const { return etat; }
    bool estEnPhaseAtterrissage() const;
    void setDestination(double x, double y);  
    void setEnDeplacement(bool etat);
    std::string getIdaeroport(Aeroport* aeroport) const;
    bool getParkingAttribue() const;
    void setParkingAttribue(bool etat);
    sf::Angle inclinaison();
    bool estBienAuSol();
    void setBienAuSol();
};

class Controleur : public Agent {
protected:
    std::mutex mutex;
    std::vector<std::unique_ptr<Avion>> avionsSousControle;   
    Temps& tempsRef;

public:
    Controleur(const std::string& id, Temps& temps);
    
    Controleur(const Controleur&) = delete;
    Controleur& operator=(const Controleur&) = delete;
    
    virtual void recevoirAvion(std::unique_ptr<Avion> avion);   
    virtual void libererAvion(const std::string& avionId);   
    virtual void run() override = 0;
};

class ControleurApproche : public Controleur {
private:
    Controleur* tour; 
    std::vector<Avion*> fileAttente;

public:
    ControleurApproche(const std::string& id, Controleur* tour, Temps& temps);
    
    ControleurApproche(const ControleurApproche&) = delete;
    ControleurApproche& operator=(const ControleurApproche&) = delete;
    
    void assignerTrajectoire(Avion* avion);
    void gererUrgence(Avion* avion);
    void demanderAutorisationAtterrissage(Avion* avion);
    void run() override;
};

class CentreControleRegional : public Controleur {
private:
    std::vector<ControleurApproche*> approchesLiees;
    double positionX;
    double positionY;

public:
    CentreControleRegional(const std::string& id, Temps& temps);
    
    CentreControleRegional(const CentreControleRegional&) = delete;
    CentreControleRegional& operator=(const CentreControleRegional&) = delete;
    
    void ajouterApproche(ControleurApproche* app);
    void transfererVol(const std::string& avionId, ControleurApproche* app);   
    void run() override;
    double getPositionX();
    double getPositionY();
    void setPositionX(double position);    
    void setPositionY(double position);
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
    TourControle(const std::string& id, int nbParkings, Temps& temps);
    
    TourControle(const TourControle&) = delete;
    TourControle& operator=(const TourControle&) = delete;
    
    bool autoriserAtterrissage(Avion* avion);
    bool autoriserDecollage(Avion* avion);
    void libererPiste();
    friend void attribuerParking(Avion* avion, TourControle * tourcontrole);
    void run() override;
};

class Monde {
private:
    std::vector<std::unique_ptr<Avion>> avions;
    std::unique_ptr<CentreControleRegional> ccr;
    std::unique_ptr<ControleurApproche> app;
    std::unique_ptr<TourControle> twr;
    InterfaceGraphique interface;
    Temps temps;

public:
    Monde();
    void initialiser();
    void demarrerSimulation();
    void arreterSimulation();
    void ajouterAvion(std::unique_ptr<Avion> avion);
    Temps& getTemps() { return temps; }
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

class Simulation {
private:
    std::string path_image;
    
public:
    Simulation();
    void executer();
};
