#include <string>
#include <thread>
#include <mutex>
#include <vector>
#include <fstream>
#include <memory>
#include <SFML/Graphics.hpp>
#include <chrono>
#include <cstdlib>
#include <ctime>

class Temps {
private:
    double facteurTemps;
    const double facteur_acceleration = 0.2;
    const double facteur_max = 5.0;
    const double facteur_min = 0.1;
    int minute = 0;
    int heure = 12;
    std::mutex mutex;
    
    std::chrono::steady_clock::time_point dernierUpdate;
    float tempsAccumule = 0.0f;
    
public:
    Temps() : facteurTemps(1) {
        dernierUpdate = std::chrono::steady_clock::now();
    }
    
    double getFacteurTemps();
    int getMinute();
    int getHeure();
    void setMinute(int monminute);
    void setHeure(int monheure);
    void accelererTemps();
    void ralentirTemps();
    void resetTemps();
    
    void update();
    
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
    std::vector<std::string> parking = {"Rien"};

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
    bool estgaré = true;
    sf::Angle angle;
    Temps& tempsRef;
    bool enDeviation;
    double destinationOriginaleX, destinationOriginaleY;
    double pointDeviationX, pointDeviationY;
    bool aAtteintPointDeviation;
    std::mutex mutexDeviation;
    
    void preparerRedecollage();
    void choisirNouvelleDestination();

public:
    Aeroport * destination;
    Avion(const std::string& id, const std::string& compagnie, Aeroport & aeroport, Temps& temps);
    ~Avion();
    bool volDemarre = false;
    void setPosition(double x, double y, double z);
    void setVitesse(double nouvelleVitesse);    

    void setEtat(const std::string& nouvelEtat);
    bool estEnAttente() const { return etat == "en attente"; }     
    Avion(const Avion&) = delete;
    Avion& operator=(const Avion&) = delete;
    void tourner();
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
    void setDestination(Aeroport & aeroport);  
    void setEnDeplacement(bool etat);
    std::string getIdaeroport(Aeroport* aeroport) const;
    bool getParkingAttribue() const;
    void setParkingAttribue(bool etat);
    sf::Angle inclinaison();
    bool estBienAuSol();
    void setBienAuSol();
    bool estgare;
    bool redemarrageProgramme = false;
    bool tourne = false;
    bool verifierRisqueCollision(const Avion* autreAvion) const;
    void calculerDeviation(const Avion* avionConflictuel);
    void appliquerDeviation();
    bool estEnDeviation() const { return enDeviation; }
};

class TourControle {
private:
    Aeroport &aeroport;
    std::string id;
    int rayon;
public:
    TourControle(Aeroport & aeroport, Avion & avion)  : aeroport(aeroport), id(aeroport.getId()) {};

    bool gererGarer(std::unique_ptr<Avion>& avion);
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
        
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        
        std::vector<std::pair<std::string, size_t>> configurations = {
            {"10", 0}, {"20", 8}, {"30", 7}, {"40", 5}, 
            {"50", 4}, {"60", 3}, {"70", 2}, {"80", 1}, 
            {"90", 9}, {"100", 6}, 
        };
        
        for (const auto& config : configurations) {
            tous_les_avions.push_back(
                std::make_unique<Avion>(
                    config.first, 
                    "AirTest" + config.first, 
                    tous_les_aeroports[config.second], 
                    monde->getTemps()
                )
            );
        }
        
        for (size_t i = 0; i < tous_les_aeroports.size(); i++) {
            tous_les_tours_de_controles.emplace_back(
                tous_les_aeroports[i], 
                *(tous_les_avions[0])
            );
        }
    }
    
    size_t genererDestinationAleatoire(size_t aeroportDepart) {
        size_t destination;
        do {
            destination = std::rand() % tous_les_aeroports.size();
        } while (destination == aeroportDepart); 
        
        return destination;
    }
    
    void demarrerTousLesAvions() {
        std::vector<size_t> aeroportsDepart;
        for (const auto& avion : tous_les_avions) {
            for (size_t i = 0; i < tous_les_aeroports.size(); i++) {
                if (avion->getPositionX() == tous_les_aeroports[i].getPositionX() + 20 &&
                    avion->getPositionY() == tous_les_aeroports[i].getPositionY() + 20) {
                    aeroportsDepart.push_back(i);
                    break;
                }
            }
        }
        
        for (size_t i = 0; i < tous_les_avions.size(); i++) {
            size_t destinationIndex = genererDestinationAleatoire(aeroportsDepart[i]);
            tous_les_avions[i]->start();
            tous_les_avions[i]->decollage();
            tous_les_avions[i]->setDestination(tous_les_aeroports[destinationIndex]);
        }
    }
    
    Aeroport* trouverAeroportAleatoire(Aeroport* exclu) {
        if (tous_les_aeroports.empty()) return nullptr;
        
        int index;
        do {
            index = std::rand() % tous_les_aeroports.size();
        } while (&tous_les_aeroports[index] == exclu);
        
        return &tous_les_aeroports[index];
    }
    
    void gererRedecollages() {
        for (auto& avion : tous_les_avions) {
            if (avion->redemarrageProgramme && avion->getEtat() == "au sol") {

                
                Aeroport* nouvelleDestination = trouverAeroportAleatoire(avion->destination);
                if (nouvelleDestination) {
                    avion->setDestination(*nouvelleDestination);
                    avion->decollage();
                    avion->redemarrageProgramme = false;
                    avion->setEnDeplacement(true);
                }
            }
        }
    }
};
