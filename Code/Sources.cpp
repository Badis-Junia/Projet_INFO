#include "Sources.hpp"
#include <chrono>
#include <iostream>
#include <algorithm>
#include <math.h>
#include <SFML/Graphics.hpp>
#include <vector>

Agent::Agent(const int & id) : id(id), actif(false) {}

std::ostream& operator<<(std::ostream& flux, const Temps& h) {
    flux << h.heure << ":" << h.minute << "H";
    return flux;
}

double Temps::getFacteurTemps() {
    this->mutex.lock();      
    double value = facteurTemps;
    this->mutex.unlock();    
    return value;
}

int Temps::getMinute() {
    return this->minute;
}

int Temps::getHeure() {
    return this->heure;
}

void Temps::accelererTemps() {
    this->mutex.lock();
    this->facteurTemps = std::min(this->facteurTemps + 0.2, this->facteur_max);
    this->mutex.unlock();
}

void Temps::ralentirTemps() {
    mutex.lock();
    facteurTemps = std::max(this->facteurTemps - 0.2, this->facteur_min);
    mutex.unlock();
}

void Temps::resetTemps() {
    this->mutex.lock();
    facteurTemps = 1.0;
    this->mutex.unlock();
}

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

Avion::Avion(const std::string& id, const std::string& compagnie, Aeroport & aeroport, Temps& temps) 
    : Agent(std::stoi(id)), tempsRef(temps) {
    this->compagnie = compagnie;
    this->positionX = 0;
    this->positionY = 0;
    this->positionZ = 0;  
    this->vitesse = this->vitesseNormal;  
    this->etat = "au sol";
    this->bienausol = false;
    this->urgence = false;
    this->carburant = 100;
    this->consommation = 0.1;
    this->enDeplacement = false;  
    this->destinationX = 0;
    this->destinationY = 0;
    this->altitudeCible = 0;
    this->penteApproche = 0;
    this->enApprocheFinale = false; 
    this->setVitesse(1.0); 
    this->setPosition(aeroport.getPositionX() + 20, aeroport.getPositionY() + 20, 0);
}

Avion::~Avion() {
    stop();
}

void Avion::run() {
   while (this->actif) {
        double facteur = tempsRef.getFacteurTemps();        
        if (this->etat == "en vol" && this->enDeplacement) {
            double directionX = this->destinationX - this->positionX;
            double directionY = this->destinationY - this->positionY;
            double distanceHorizontale = sqrt(directionX * directionX + directionY * directionY);

            if (distanceHorizontale > 0) {
                directionX /= distanceHorizontale;
                directionY /= distanceHorizontale;
            }

            if (distanceHorizontale > this->distanceAtterissage) {
                this->vitesse = this->vitesseNormal;
            } 
            else if (distanceHorizontale > 80.0) {
                this->vitesse = this->vitesseApproche;
                if (!this->enApprocheFinale) {
                    this->enApprocheFinale = true;
                }
            }
            else if (distanceHorizontale > 40.0) {
                this->vitesse = this->vitesseFinale;
            }
            else if (distanceHorizontale > 20.0) {
                this->vitesse = this->vitesseAtterrissage;
            }
            else {
                this->vitesse = 1.0;
            }

            double vitesseDeplacement = this->vitesse * 0.02 * facteur;
            
            this->positionX += directionX * vitesseDeplacement;
            this->positionY += directionY * vitesseDeplacement;

            double distanceRestante = sqrt(pow(this->destinationX - this->positionX, 2) +
                                         pow(this->destinationY - this->positionY, 2));

            if (distanceRestante > 50.0) {
                double progression = (200.0 - distanceRestante) / 100.0;
                this->positionZ = 300 - (progression * 250);  
                if (this->positionZ < 50) this->positionZ = 50;
            }
            else if (distanceRestante > 25.0) {
                double progression = (100.0 - distanceRestante) / 50.0;
                this->positionZ = 50 - (progression * 40);  
                if (this->positionZ < 10) this->positionZ = 10;
            }
            else {
                this->positionZ = distanceRestante * 0.2;  
                if (this->positionZ < 5) this->positionZ = 5;
            }

            if (distanceRestante < 25.0 && this->positionZ <= 50) {
                this->etat = "atterrissage";
            } else if (distanceRestante < 5.0 && this->positionZ <= 5) {
                this->positionX = this->destinationX;
                this->positionY = this->destinationY;
                this->positionZ = 0;
                this->enDeplacement = false;
                this->etat = "au sol";
                this->vitesse = 0;
                this->enApprocheFinale = false;
            }

            consommerCarburant("en vol");
        }
        else if (this->etat == "decollage") {
            this->positionZ += 10 * facteur;  
            this->positionY += 1 * facteur;
            if (this->positionZ >= 300) {  
                this->etat = "en vol";
            }
            consommerCarburant("decollage");
        }
        else if (this->etat == "atterrissage") {
            double directionX = this->destinationX - this->positionX;
            double directionY = this->destinationY - this->positionY;
            double distanceRestante = sqrt(directionX * directionX + directionY * directionY);

            if (distanceRestante > 0) {
                directionX /= distanceRestante;
                directionY /= distanceRestante;
            }

            double vitesseDeplacement = this->vitesse * 0.03 * facteur;
            this->positionX += directionX * vitesseDeplacement;
            this->positionY += directionY * vitesseDeplacement;

            if (this->positionZ > 50) {
                this->positionZ -= 15 * facteur;
            } else if (this->positionZ > 10) {
                this->positionZ -= 8 * facteur;
            } else {
                this->positionZ -= 3 * facteur;
            }
            
            if (this->positionZ < 0) this->positionZ = 0;

            if (distanceRestante < 5.0 && this->positionZ <= 5) {
                this->positionX = this->destinationX;
                this->positionY = this->destinationY;
                this->positionZ = 0;
                this->enDeplacement = false;
                this->etat = "au sol";
            }

            consommerCarburant("atterrissage");
        }
        else if (this->etat == "au sol") {
            this->positionZ = 0;
            this->enDeplacement = false;
        }

        if (this->carburant < 20 && !this->urgence && this->etat != "au sol") {
            declarerUrgence(true);
            std::cout << "Avion " << this->id << " déclare une urgence carburant !" << std::endl;
        }

        if (this->urgence && this->etat != "atterrissage" && this->etat != "au sol") {
            this->etat = "atterrissage";
            this->enDeplacement = true;
            std::cout << "Avion " << this->id << " entame un atterrissage d'urgence !" << std::endl;
        }

        if (this->carburant <= 0 && this->etat != "au sol") {
            this->etat = "crash";
            this->enDeplacement = false;
            std::cout << "Avion " << this->id << " a crashé ! Plus de carburant." << std::endl;
        }

        int delai = static_cast<int>(100 / facteur);
        std::this_thread::sleep_for(std::chrono::milliseconds(delai));
    }
}

void Avion::majPosition() {
    double facteur = tempsRef.getFacteurTemps();
    this->positionX += this->vitesse * 0.05 * facteur;
    this->positionY += this->vitesse * 0.05 * facteur;
    this->positionZ += this->vitesse * 2 * facteur;

    if(this->etat == "au sol") {
        this->positionZ = 0;
    }

    if (this->etat == "atterissage") {
        if (this->positionZ <= 0) {
            this->positionZ = 0;
            this->etat = "au sol";
            this->enDeplacement = false; 
        }
    }
}

bool Avion::estBienAuSol() {
    return this->bienausol;
}

void Avion::setBienAuSol() {
    this->bienausol = true;
}

bool Avion::estEnPhaseAtterrissage() const {
    if (!this->enDeplacement) return false;
    
    double distanceHorizontale = sqrt(pow(this->destinationX - this->positionX, 2) +
                                     pow(this->destinationY - this->positionY, 2));
    return distanceHorizontale <= this->distanceAtterissage;
}

void Avion::setDestination(double x, double y) {
    this->destinationX = x + 30;
    this->destinationY = y + 30;
    this->enDeplacement = true;
    this->enApprocheFinale = false; 

    if(this->etat != "decollage") {
        this->etat = "en vol";
    }
}

bool Avion::getParkingAttribue() const{
    return this->estgare;
}

void Avion::setParkingAttribue(bool etat){
    this->estgare=etat;
}

void Avion::setEnDeplacement(bool etat) {
    this->enDeplacement = etat;
}

sf::Angle Avion::inclinaison(){
    double dx = this->destinationX - this->positionX;
    double dy = this->destinationY - this->positionY;
    double rad = std::atan2(dy, dx);
    double deg = rad * (180.0 / M_PI);

    deg += 90.0;

    if (deg < 0) {
        deg += 360.0;
    }

    this->angle = sf::degrees(deg);
    return this->angle;
}

void Avion::consommerCarburant(std::string etat) {
    double facteur = tempsRef.getFacteurTemps();
    
    if(this->etat == "en vol") {
        this->carburant -= this->consommation * facteur;
    } else if(this->etat == "decollage"){
        this->carburant -= 1 * facteur;
    }

    if (carburant < 0) {
        this->carburant = 0;
    }
}

void Avion::atterrissage() {
    if (this->etat != "atterrissage" && this->etat != "au sol") {
        this->etat = "atterrissage";
        this->enDeplacement = false;
        std::cout << "Avion " << this->id << " commence l'atterrissage" << std::endl;
    }
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

std::string Avion::getIdaeroport(Aeroport* aeroport) const{
    return aeroport->getId();
}

void Aeroport::setPosition(double positionX, double positionY) {
    this->positionX = positionX;
    this->positionY = positionY;
}

double Aeroport::getPositionX() {
    return this->positionX;
}

double Aeroport::getPositionY() {
    return this->positionY;
}

std::string Aeroport::getId() {
    return this->id;
}

Controleur::Controleur(const std::string& id, Temps& temps) : Agent(std::stoi(id)), tempsRef(temps) {}

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

ControleurApproche::ControleurApproche(const std::string& id, Controleur* tour, Temps& temps) 
    : Controleur(id, temps), tour(tour) {}

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
        double facteur = tempsRef.getFacteurTemps();
        std::lock_guard<std::mutex> lock(mutex);
        for (auto& avion : this->avionsSousControle) {
            if (avion->estEnUrgence()) {
                gererUrgence(avion.get());
            } else {
                assignerTrajectoire(avion.get());
                demanderAutorisationAtterrissage(avion.get());
            }
        }
        int delai = static_cast<int>(500 / facteur);
        std::this_thread::sleep_for(std::chrono::milliseconds(delai));
    }
}

CentreControleRegional::CentreControleRegional(const std::string& id, Temps& temps) : Controleur(id, temps) {}

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
        double facteur = tempsRef.getFacteurTemps();
        int delai = static_cast<int>(1000 / facteur);
        std::this_thread::sleep_for(std::chrono::milliseconds(delai));
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

TourControle::TourControle(const std::string& id, int nbParkings, Temps& temps) 
    : Controleur(id, temps), pisteLibre(true) {
    for (int i = 1; i <= nbParkings; ++i) {
        parkings["Place " + std::to_string(i)] = true;
    }
}

bool TourControle::autoriserAtterrissage(Avion* avion) {
    std::lock_guard<std::mutex> lock(mutexPiste);
    if (this->pisteLibre) {
        this->pisteLibre = false;
        std::cout << "Autorisation d'atterrissage pour avion " << avion->getId() << std::endl;
        return true;
    }
    return false;
}

bool TourControle::autoriserDecollage(Avion* avion) {
    std::lock_guard<std::mutex> lock(mutexPiste);
    if (this->pisteLibre) {
        this->pisteLibre= false;
        std::cout << "Autorisation de décollage pour avion " << avion->getId() << std::endl;
        return true;
    }
    return false;
}

void TourControle::libererPiste() {
    std::lock_guard<std::mutex> lock(mutexPiste);
    pisteLibre = true;
}

void attribuerParking(Avion* avion, TourControle* tourcontrole) {
    for (auto& parking : tourcontrole->parkings) {
        if (parking.second) {
            parking.second = false;
            avion->setParkingAttribue(true);
            std::cout << "Parking " << parking.first << " attribué à avion " << avion->getId() << std::endl;
            return;
        }
    }
    std::cout << "Aucun parking disponible pour avion " << avion->getId() << std::endl;
}

void TourControle::run() {
    while (this->actif) {
        double facteur = tempsRef.getFacteurTemps();
        std::lock_guard<std::mutex> lock(mutex);
        for (auto& avion : this->avionsSousControle) {
            if (avion->getEtat() == "au sol" && !avion->getParkingAttribue()) {
                attribuerParking(avion.get(), this);
            }
        }
        int delai = static_cast<int>(200 / facteur);
        std::this_thread::sleep_for(std::chrono::milliseconds(delai));
    }
}

void InterfaceGraphique::afficherAPP(const std::vector<Avion*>& avions) {}
void InterfaceGraphique::afficherTWR(const std::vector<Avion*>& avions) {}
void InterfaceGraphique::afficherCCR(const std::vector<Avion*>& avions) {}

Monde::Monde() {}

void Monde::initialiser() {
    ccr = std::make_unique<CentreControleRegional>("1", temps);
    twr = std::make_unique<TourControle>("2", 5, temps);
    app = std::make_unique<ControleurApproche>("3", twr.get(), temps);
    
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

Simulation::Simulation() : path_image("../Pictures/") {}

void Simulation::executer() {
    Monde monde;
    monde.initialiser();
    monde.demarrerSimulation();
    const sf::Vector2u WINDOW_SIZE(1300, 805);
    sf::RenderWindow app(sf::VideoMode({WINDOW_SIZE.x, WINDOW_SIZE.y}, 32), "Projet_INFO");
    app.setFramerateLimit(60);

    sf::Texture backgroundImage, avionTexture, aeroportTexture;
    sf::Font font;
    if (!backgroundImage.loadFromFile(path_image + "background.png") || 
        !avionTexture.loadFromFile(path_image + "avion.png") || 
        !aeroportTexture.loadFromFile(path_image + "aeroport.png") || !font.openFromFile(path_image + "arial.ttf")) {
        throw std::runtime_error("Erreur pendant le chargement des images");
    }

    sf::Sprite backgroundSprite(backgroundImage), avionSprite(avionTexture);
    std::vector<sf::Sprite> aeroportsSprite;

    std::vector<Aeroport> aeroports = {
        {"Oregon", 100, 200}, {"Texas", 450, 600}, {"Ohio", 925, 380},
        {"Montana", 390, 180}, {"Colorado", 410, 425}, {"NewYork", 1100, 230},
        {"Tennessy", 900, 500}, {"Floride", 1025, 750}, {"Californie", 175, 450},
        {"Iowa", 700, 350}
    };

    for (size_t i = 0; i < aeroports.size(); i++) {
        sf::Sprite aeroportSprite(aeroportTexture);
        aeroportSprite.setScale(sf::Vector2f(0.12, 0.12));
        aeroportSprite.setPosition(sf::Vector2f(aeroports[i].getPositionX(), aeroports[i].getPositionY()));
        aeroportsSprite.push_back(aeroportSprite);
    }

    Aeroport aeroportDepart = aeroports[0];
    Avion avionTest("10", "AirTest", aeroportDepart, monde.getTemps());
    avionSprite.setScale(sf::Vector2f(0.6, 0.6));
    
    avionTest.start();
    avionTest.decollage();
    avionTest.setDestination(aeroports[8].getPositionX(), aeroports[8].getPositionY());
    int counter = 0;
    Journal journal("monlog.txt");

    
    sf::Text texteFacteurTemps(font);
    texteFacteurTemps.setFont(font);
    texteFacteurTemps.setCharacterSize(30);
    texteFacteurTemps.setFillColor(sf::Color::White);
    texteFacteurTemps.setPosition(sf::Vector2f(static_cast<float>(10), static_cast<float>(0)));
    
    while (app.isOpen()) {
        while (std::optional event = app.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                app.close();
            }
            if (auto* keyEvent = event->getIf<sf::Event::KeyPressed>()) {
                if (keyEvent->code == sf::Keyboard::Key::Enter) {
                    app.close();
                }
                else if (keyEvent->code == sf::Keyboard::Key::I) {
                    monde.getTemps().accelererTemps();
                }
                else if (keyEvent->code == sf::Keyboard::Key::K) {
                    monde.getTemps().ralentirTemps();
                }
            }
        }

        texteFacteurTemps.setString("Vitesse du temps: " + std::to_string(monde.getTemps().getFacteurTemps()).substr(0, 3) + "x");

        if (!avionTest.volDemarre) {
            avionSprite.setRotation(avionTest.inclinaison());
            if (avionTest.volDemarre && avionTest.getEtat() == "en vol" && 
                sqrt(pow(avionTest.getPositionX() - aeroportDepart.getPositionX(), 2) + 
                     pow(avionTest.getPositionY() - aeroportDepart.getPositionY(), 2)) < 50) {
                avionTest.atterrissage();
            }
            avionTest.volDemarre = true;
            std::cout << "Position avion: (" << avionTest.getPositionX() << ", " 
                      << avionTest.getPositionY() << ", " << avionTest.getPositionZ() << ")" 
                      << " - carburant: " << avionTest.getCarburant() 
                      << " - état: " << "vol vers " << aeroportDepart.getId() << std::endl;

        }

        if(avionTest.getEtat() != "au sol") {
            avionSprite.setPosition(sf::Vector2f(static_cast<float>(avionTest.getPositionX()), 
                                                static_cast<float>(avionTest.getPositionY())));
        }

        if (counter++ % 60 == 0) {
            if(!avionTest.estBienAuSol()) {
                std::cout << "Position avion: (" << avionTest.getPositionX() << ", " 
                          << avionTest.getPositionY() << ", " << avionTest.getPositionZ() << ")" 
                          << " - carburant: " << avionTest.getCarburant() 
                          << " - état: " << avionTest.getEtat() << std::endl;
                journal.log("Position avion:" + std::to_string(avionTest.getPositionX()) + "," + 
                           std::to_string(avionTest.getPositionY()) + "," + 
                           std::to_string(avionTest.getPositionZ()) + 
                           " - carburant:" + std::to_string(avionTest.getCarburant()) +
                           " - état:" + avionTest.getEtat());

                if(avionTest.getEtat() == "au sol") {
                    avionTest.setBienAuSol();
                    avionSprite.setPosition(sf::Vector2f(static_cast<float>(10000), static_cast<float>(10000)));
                }
            }
        }

        app.clear();
        app.draw(backgroundSprite);
        app.draw(avionSprite);
        
        for (auto& aeroportSprite : aeroportsSprite) {
            app.draw(aeroportSprite);
        }
        
        if (font.getInfo().family != "") {
            app.draw(texteFacteurTemps);
        }
        
        app.display();
    }

    avionTest.stop();
    monde.arreterSimulation();
}
