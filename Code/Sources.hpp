#include <string>
#include <thread>
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
    virtual ~Agent() = default;
    
      
    Agent(const Agent&) = delete;
    Agent& operator=(const Agent&) = delete;
    
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
    bool enDeplacement;  
    double destinationX, destinationY;  
                                        
public:
    Avion(const std::string& id, const std::string& compagnie);
    ~Avion();

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
    void consommerCarburant();
    double getPositionX() const;
    double getPositionY() const;
    double getPositionZ() const { return positionZ; }
    std::string getEtat() const { return etat; }

    void setDestination(double x, double y);  
    void setEnDeplacement(bool etat);         
};

 




class Controleur : public Agent {
protected:
    std::mutex mutex;
    std::vector<std::unique_ptr<Avion>> avionsSousControle;   

public:
    Controleur(const std::string& id);
    
      
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
    ControleurApproche(const std::string& id, Controleur* tour);
    
      
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
    CentreControleRegional(const std::string& id);
    
      
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
    TourControle(const std::string& id, int nbParkings);
    
      
    TourControle(const TourControle&) = delete;
    TourControle& operator=(const TourControle&) = delete;
    
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
