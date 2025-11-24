#include "Sources.hpp"
#include <chrono>
#include <iostream>
#include <algorithm>
#include <math.h>

Agent::Agent(const int & id) : id(id), actif(false) {}

void Agent::start() {
    this->actif = true;
    this->monthread = std::thread(&Agent::run, this);
}

void Agent::stop() {
    this->actif = false;
    if (this->monthread.joinable()) {
        this->monthread.join();
    }
}

std::string Agent::getId() const {
    return std::to_string(this->id);
}

 



Avion::Avion(const std::string& id, const std::string& compagnie) : Agent(std::stoi(id)) {
    this->compagnie = compagnie;
    this->positionX = 0;
    this->positionY = 0;
    this->positionZ = 10000;
    this->vitesse = 800;
    this->etat = "en vol";
    this->urgence = false;
    this->carburant = 100;
    this->consommation = 0.1;
    this->enDeplacement = false;  
    this->destinationX = 0;
    this->destinationY = 0;
}

Avion::~Avion() {
    stop();
}

void Avion::run() {
    while (this->actif) {
       
        if (this->enDeplacement && this->etat == "en vol") {
            
            double directionX = this->destinationX - this->positionX;
            double directionY = this->destinationY - this->positionY;
            
            
            double distance = sqrt(directionX * directionX + directionY * directionY);
            if (distance > 0) {
                directionX /= distance;
                directionY /= distance;
            }
            
            
            double vitesseDeplacement = this->vitesse * 0.1;
            this->positionX += directionX * vitesseDeplacement;
            this->positionY += directionY * vitesseDeplacement;
            
            
            double distanceRestante = sqrt((this->destinationX - this->positionX) * (this->destinationX - this->positionX) + 
                                         (this->destinationY - this->positionY) * (this->destinationY - this->positionY));
            
            if (distanceRestante < 10.0) { 
                this->positionX = this->destinationX;
                this->positionY = this->destinationY;
                this->enDeplacement = false;
                this->atterrissage(); 
            }
        }
        
        else if (this->etat == "au sol") {
            this->enDeplacement = false; 
        }
        else {
            
            majPosition();
        }
        
        consommerCarburant();
        
        if (this->carburant < 20 && !this->urgence) {
            declarerUrgence(true);
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void Avion::majPosition() {
    this->positionX += this->vitesse * 0.1;
    this->positionY += this->vitesse * 0.05;
    
    if (this->etat == "atterissage") {
        this->positionZ -= 500;
        if (this->positionZ <= 0) {
            this->positionZ = 0;
            this->etat = "au sol";
            this->enDeplacement = false; 
        }
    }
}

void Avion::setDestination(double x, double y) {
    this->destinationX = x;
    this->destinationY = y;
    this->enDeplacement = true;
    this->etat = "en vol";
}

void Avion::setEnDeplacement(bool etat) {
    this->enDeplacement = etat;
}

void Avion::consommerCarburant() {
    this->carburant -= this->consommation;
    if (carburant < 0) {
        this->carburant = 0;
    }
}

void Avion::atterrissage() {
    this->etat = "atterissage";

    this->enDeplacement = false;
}

void Avion::decollage() {
    this->etat = "decollage";
    this->positionZ = 0;
}

void Avion::declarerUrgence(bool etatUrgence) {
    this->urgence = etatUrgence;
    if(etatUrgence) {
        this->etat = "urgence ! ";
    } 
}

bool Avion::estEnUrgence() const {
    return this->urgence;
}

double Avion::getCarburant() const {
    return this->carburant;
}

double Avion::getPositionX() const {
    return this->positionX;
}

double Avion::getPositionY() const {
    return this->positionY;
}


void Avion::setPosition(double x, double y, double z) {
    this->positionX = x;
    this->positionY = y;
    this->positionZ = z;
}

void Avion::setVitesse(double nouvelleVitesse) {
    this->vitesse = nouvelleVitesse;
} 




 
Controleur::Controleur(const std::string& id) : Agent(std::stoi(id)) {}

void Controleur::recevoirAvion(std::unique_ptr<Avion> avion) {
    std::lock_guard<std::mutex> lock(mutex);
    this->avionsSousControle.push_back(std::move(avion));
}

void Controleur::libererAvion(const std::string& avionId) {
    std::lock_guard<std::mutex> lock(mutex);
    avionsSousControle.erase(
        std::remove_if(avionsSousControle.begin(), avionsSousControle.end(),
            [&](const std::unique_ptr<Avion>& avion) {
                return avion->getId() == avionId;
            }),
        avionsSousControle.end()
    );
}


 



 
ControleurApproche::ControleurApproche(const std::string& id, Controleur* tour) 
    : Controleur(id), tour(tour) {}

void ControleurApproche::assignerTrajectoire(Avion* avion) {
    std::cout << "Trajectoire assignée à l'avion " << avion->getId() << std::endl;
}

void ControleurApproche::gererUrgence(Avion* avion) {
    std::cout << "URGENCE: Gestion de l'avion " << avion->getId() << std::endl;
    avion->atterrissage();
}

void ControleurApproche::demanderAutorisationAtterrissage(Avion* avion) {
    if (avion->getPositionZ() <= 3000 && avion->getCarburant() > 10) {
        std::cout << "Autorisation d'atterrissage demandée pour avion " << avion->getId() << std::endl;
    }
}

void ControleurApproche::run() {
    while (this->actif) {
        std::lock_guard<std::mutex> lock(mutex);
        for (auto& avion : this->avionsSousControle) {
            if (avion->estEnUrgence()) {
                gererUrgence(avion.get());
            } else {
                assignerTrajectoire(avion.get());
                demanderAutorisationAtterrissage(avion.get());
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}


 



 
CentreControleRegional::CentreControleRegional(const std::string& id) : Controleur(id) {}

void CentreControleRegional::ajouterApproche(ControleurApproche* app) {
    this->approchesLiees.push_back(app);
}

void CentreControleRegional::transfererVol(const std::string& avionId, ControleurApproche* app) {
    std::unique_ptr<Avion> avionToTransfer;
    
    {
        std::lock_guard<std::mutex> lock(mutex);
        auto it = std::find_if(avionsSousControle.begin(), avionsSousControle.end(),
            [&](const std::unique_ptr<Avion>& avion) {
                return avion->getId() == avionId;
            });
            
        if (it != avionsSousControle.end()) {
            avionToTransfer = std::move(*it);
            avionsSousControle.erase(it);
        }
    }
    
    if (avionToTransfer) {
        app->recevoirAvion(std::move(avionToTransfer));
    }
}

void CentreControleRegional::run() {
    while (this->actif) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

double CentreControleRegional::getPositionX() {
    return this->positionX;
}
    
double CentreControleRegional::getPositionY() {
    return this->positionY;
}

void CentreControleRegional::setPositionX(double position) {
    this->positionX = position;
}
    
void CentreControleRegional::setPositionY(double position) {
    this->positionY = position;
}
 




 
TourControle::TourControle(const std::string& id, int nbParkings) : Controleur(id), pisteLibre(true) {
    for (int i = 1; i <= nbParkings; ++i) {
        parkings["Place " + std::to_string(i)] = true;
    }
}

bool TourControle::autoriserAtterrissage(Avion* avion) {
    std::lock_guard<std::mutex> lock(mutexPiste);
    if (pisteLibre) {
        pisteLibre = false;
        std::cout << "Autorisation d'atterrissage pour avion " << avion->getId() << std::endl;
        return true;
    }
    return false;
}

bool TourControle::autoriserDecollage(Avion* avion) {
    std::lock_guard<std::mutex> lock(mutexPiste);
    if (pisteLibre) {
        pisteLibre = false;
        std::cout << "Autorisation de décollage pour avion " << avion->getId() << std::endl;
        return true;
    }
    return false;
}

void TourControle::libererPiste() {
    std::lock_guard<std::mutex> lock(mutexPiste);
    pisteLibre = true;
}

void TourControle::attribuerParking(Avion* avion) {
    for (auto& parking : parkings) {
        if (parking.second) {
            parking.second = false;
            std::cout << "Parking " << parking.first << " attribué à avion " << avion->getId() << std::endl;
            return;
        }
    }
    std::cout << "Aucun parking disponible pour avion " << avion->getId() << std::endl;
}

void TourControle::run() {
    while (this->actif) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

 




 
void InterfaceGraphique::afficherAPP(const std::vector<Avion*>& avions) {}
void InterfaceGraphique::afficherTWR(const std::vector<Avion*>& avions) {}
void InterfaceGraphique::afficherCCR(const std::vector<Avion*>& avions) {}

 

 



Monde::Monde() {}

void Monde::initialiser() {
    ccr = std::make_unique<CentreControleRegional>("1");
    twr = std::make_unique<TourControle>("2", 5);
    app = std::make_unique<ControleurApproche>("3", twr.get());
    
    ccr->ajouterApproche(app.get());
}

void Monde::demarrerSimulation() {
    ccr->start();
    app->start();
    twr->start();
}

void Monde::arreterSimulation() {
    twr->stop();
    app->stop();
    ccr->stop();
}

void Monde::ajouterAvion(std::unique_ptr<Avion> avion) {
    avions.push_back(std::move(avion));
}


 



 
Journal::Journal(const std::string& nomFichier) {
    fichier.open(nomFichier);
}

void Journal::log(const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex);
    if (fichier.is_open()) {
        fichier << message << std::endl;
    }
}

Journal::~Journal() {
    if (fichier.is_open()) {
        fichier.close();
    }
}
