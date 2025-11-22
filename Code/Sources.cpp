#include "Sources.hpp"
#include <chrono>


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
}


Avion::~Avion() {
    stop();
}

void Avion::run() {
    while (this->actif) {
        majPosition();
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
            this->etat = "au sol.";
        }
    }
}

void Avion::consommerCarburant() {
    this->carburant -= this->consommation;
    if (carburant < 0) {
        this->carburant = 0;
    }
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






Controleur::Controleur(const std::string& id) : Agent(std::stoi(id)) {}

void Controleur::recevoirAvion(Avion avion) {
    this->mutex.lock();                 
    this->avionsSousControle.push_back(avion);
    this->mutex.unlock();               
}

void Controleur::libererAvion(Avion* avion) {
    this->mutex.lock();
    for (auto i = this->avionsSousControle.begin(); i != this->avionsSousControle.end(); ) {
        if (i->getId() == avion->getId()) {
            i = this->avionsSousControle.erase(i);
        } else {
            i++;
        }
    }
    this->mutex.unlock();               
}

ControleurApproche::ControleurApproche(const std::string& id, Controleur* tour) : Controleur(id), tour(tour) {}

void ControleurApproche::run() {
    while (this->actif) {
        for (auto& avion : this->avionsSousControle) {
            if (avion.estEnUrgence()) {
                gererUrgence(&avion);
            } else {
                assignerTrajectoire(&avion);
                demanderAutorisationAtterrissage(&avion);
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}






TourControle::TourControle(const std::string& id, int nbParkings) : Controleur(id), pisteLibre(true) {
    for (int i = 1; i <= nbParkings; ++i) {
        parkings["P" + std::to_string(i)] = true;
    }
}

void TourControle::run() {
    while (this->actif) {
        if (this->pisteLibre && !this->fileAttenteDecollage.empty()) {
            auto avion = this->fileAttenteDecollage[0];
            if (autoriserDecollage(avion)) {
                this->fileAttenteDecollage.erase(this->fileAttenteDecollage.begin());
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}






CentreControleRegional::CentreControleRegional(const std::string& id) : Controleur(id) {}

void CentreControleRegional::run() {
    while (this->actif) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}






Monde::Monde() {}

void Monde::initialiser() {
    ccr = std::unique_ptr<CentreControleRegional>(new CentreControleRegional("1"));
    twr = std::unique_ptr<TourControle>(new TourControle("2", 5));
    app = std::unique_ptr<ControleurApproche>(new ControleurApproche("3", twr.get()));
    
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






Journal::Journal(const std::string& nomFichier) {
    fichier.open(nomFichier);
}

void Journal::log(const std::string& message) {
    this->mutex.lock();
    if (fichier.is_open()) {
        fichier << message << std::endl;
    }
    this->mutex.unlock();
}

Journal::~Journal() {
    if (fichier.is_open()) {
        fichier.close();
    }
}
