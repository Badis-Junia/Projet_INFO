#include <string>
#include <thread>
#include <mutex>
#include <vector>
#include <fstream>
#include <memory>
#include <SFML/Graphics.hpp>


class Temps {
private:
    double facteurTemps;
    const double facteur_acceleration = 0.2;
    const double facteur_max = 5.0;
    const double facteur_min = 0.1;
    int minute = 0;
    int heure = 12;
    std::mutex mutex;

public:
    Temps() : facteurTemps(1) {}
    
    double getFacteurTemps();
    int getMinute();
    int getHeure();
    void setMinute(int monminute);
    void setHeure(int monheure);
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
protected:
    std::string id;
    double positionX;
    double positionY;
    bool pistelibre;
    std::vector<std::string> parking = {"Rien"}; // , "Rien", "Rien", "Rien", "Rien", "Rien", "Rien", "Rien", "Rien", "Rien"};

public:
    Aeroport(const std::string & id, double positionX, double positionY) : id(id), positionX(positionX), positionY(positionY) {};
    std::vector<std::string> getParking();
    bool parkingvide();
    double getPositionX();
    double getPositionY();
    std::string getId();
    void setPosition(double unepositionX, double unepositionY);
    void setParking(int indice, std::string etat);
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
    bool estgare;
};

class TourControle {
    private:
        Aeroport &aeroport;
        Avion &avion;
        std::string id;
        int rayon;
    public:
        TourControle(Aeroport & aeroport, Avion & avion)  : aeroport(aeroport), avion(avion), id(aeroport.getId()) {};

        void gererGarer();
};












class Monde {
private:
    std::vector<std::unique_ptr<Avion>> avions;
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


class CentreControle {

public:
    std::vector<Aeroport> tous_les_aeroports = {
        {"Oregon", 100, 200}, {"Texas", 450, 600}, {"Ohio", 925, 380},
        {"Montana", 390, 180}, {"Colorado", 410, 425}, {"NewYork", 1100, 230},
        {"Tennessy", 900, 500}, {"Floride", 1025, 750}, {"Californie", 175, 450},
        {"Iowa", 700, 350}};
    std::vector<TourControle> tous_les_tours_de_controles;    
    std::vector<std::unique_ptr<Avion>> tous_les_avions;
    CentreControle(Monde* monde) {
        tous_les_avions.push_back(
            std::make_unique<Avion>("10", "AirTest", tous_les_aeroports[0], monde->getTemps())
        );
        tous_les_tours_de_controles.emplace_back(
            tous_les_aeroports[0], 
            *(tous_les_avions[0]) 
        );
    }


};
