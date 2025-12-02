#include "Sources.hpp"
#include <chrono>
#include <iostream>
#include <algorithm>
#include <math.h>
#include <SFML/Graphics.hpp>
#include <vector>
#include <iomanip>

Agent::Agent(const int & id) : id(id), actif(false) {}

void Agent::stop() {
    this->actif = false;
    if (this->monthread.joinable()) {
        this->monthread.join();
    }
}

std::string Agent::getId() const {
    return std::to_string(this->id);
}

void Agent::start() {
    this->actif = true;
    this->monthread = std::thread(&Agent::run, this);
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
    this->enDeviation = false;
    this->destinationOriginaleX = 0;
    this->destinationOriginaleY = 0;
    this->pointDeviationX = 0;
    this->pointDeviationY = 0;
    this->aAtteintPointDeviation = false;
    this->estgare = false;
    this->tourne = false;
    this->destination = nullptr;
}

Avion::~Avion() {
    stop();
}

bool Avion::verifierRisqueCollision(const Avion* autreAvion) const {
    if (this->id == autreAvion->id) return false;
    if (this->etat == "au sol" || autreAvion->etat == "au sol") return false;
    
    double distanceHorizontale = sqrt(pow(this->positionX - autreAvion->positionX, 2) +
                                     pow(this->positionY - autreAvion->positionY, 2));
    double distanceVerticale = abs(this->positionZ - autreAvion->positionZ);
    
    return (distanceHorizontale < 15 && distanceVerticale < 81.5);
}

void Avion::tourner() {
    static double angleTour = 0.0; 
    double facteur = tempsRef.getFacteurTemps();
    double vitesseTour = 0.02 * facteur;
    
    if (destination != nullptr) {
        double centreX = destination->getPositionX() + 20;
        double centreY = destination->getPositionY() + 20;
        double rayon = 40.0;
        
        angleTour -= vitesseTour;
        if (angleTour > 2 * M_PI) angleTour -= 2 * M_PI;
        

        double x = centreX + rayon * std::cos(angleTour);
        double y = centreY + rayon * std::sin(angleTour);
        
        this->setPosition(x, y, 20);


        double angleVersCentre = std::atan2(centreY - y, centreX - x) * (180.0 / M_PI);
        double angleVersExterieur = angleVersCentre - 180.0;
        

        while (angleVersExterieur <= 360.0) angleVersExterieur += 360.0;
        while (angleVersExterieur > 0) angleVersExterieur -= 360.0;
        
        this->angle = sf::degrees(angleVersExterieur);
    }
}

void Avion::calculerDeviation(const Avion* avionConflictuel) {
    std::lock_guard<std::mutex> lock(mutexDeviation);
    
    if (enDeviation) return;
    
    destinationOriginaleX = destinationX;
    destinationOriginaleY = destinationY;
    
    double dx = destinationX - positionX;
    double dy = destinationY - positionY;
    
    double longueur = sqrt(dx * dx + dy * dy);
    if (longueur > 0) {
        dx /= longueur;
        dy /= longueur;
    }
    
    double perpX = -dy;
    double perpY = dx;
    
    pointDeviationX = positionX + perpX * 13.0;
    pointDeviationY = positionY + perpY * 13.0;
    
    enDeviation = true;
    aAtteintPointDeviation = false;
}

void Avion::appliquerDeviation() {
    if (!enDeviation) return;
    
    std::lock_guard<std::mutex> lock(mutexDeviation);
    
    if (!aAtteintPointDeviation) {
        double dx = pointDeviationX - positionX;
        double dy = pointDeviationY - positionY;
        double distance = sqrt(dx * dx + dy * dy);
        
        if (distance > 5.0) {
            if (distance > 0) {
                dx /= distance;
                dy /= distance;
            }
            
            double facteur = tempsRef.getFacteurTemps();
            double vitesseDeplacement = this->vitesse * 0.02 * facteur;
            
            positionX += dx * vitesseDeplacement;
            positionY += dy * vitesseDeplacement;
        } else {
            aAtteintPointDeviation = true;
        }
    } else {
        double dx = destinationOriginaleX - positionX;
        double dy = destinationOriginaleY - positionY;
        double distance = sqrt(dx * dx + dy * dy);
        
        if (distance > 0) {
            dx /= distance;
            dy /= distance;
        }
        
        double facteur = tempsRef.getFacteurTemps();
        double vitesseDeplacement = this->vitesse * 0.02 * facteur;
        
        positionX += dx * vitesseDeplacement;
        positionY += dy * vitesseDeplacement;
        
        if (distance < 10.0) {
            enDeviation = false;
            aAtteintPointDeviation = false;
            destinationX = destinationOriginaleX;
            destinationY = destinationOriginaleY;
        }
    }
}

void Avion::run() {
    while (this->actif) {
        double facteur = tempsRef.getFacteurTemps();        
        
        if (enDeviation) {
            appliquerDeviation();
            consommerCarburant("en vol");
        }
        else if (this->etat == "en vol" && this->enDeplacement) {
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

            double distanceRestante = sqrt(pow(this->destinationX - this->positionX, 2) + pow(this->destinationY - this->positionY, 2));

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
                std::lock_guard<std::mutex> lock(mutexDeviation);
                this->enDeviation = false;
                this->aAtteintPointDeviation = false;
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
        else if (this->etat == "en attente") {
            tourner();
            consommerCarburant("en vol");
        }
        else if (this->etat == "au sol") {
            this->positionZ = 0;
            this->enDeplacement = false;
        }

        if (this->carburant < 20 && !this->urgence && this->etat != "au sol") {
            declarerUrgence(true);
        }

        if (this->urgence && this->etat != "atterrissage" && this->etat != "au sol") {
            this->etat = "atterrissage";
            this->enDeplacement = true;
        }

        if (this->carburant <= 0 && this->etat != "au sol") {
            this->etat = "crash";
            this->enDeplacement = false;
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

void Avion::setEtat(const std::string& nouvelEtat) {
    this->etat = nouvelEtat;
}

bool Avion::estEnPhaseAtterrissage() const {
    if (!this->enDeplacement) return false;
    
    double distanceHorizontale = sqrt(pow(this->destinationX - this->positionX, 2) +
                                     pow(this->destinationY - this->positionY, 2));
    return distanceHorizontale <= this->distanceAtterissage;
}

void Avion::setDestination(Aeroport & aeroport) {
    this->destinationX = aeroport.getPositionX() + 30;
    this->destinationY = aeroport.getPositionY() + 30;
    this->enDeplacement = true;
    this->enApprocheFinale = false; 
    this->destination = &aeroport;
    if(this->etat != "decollage") {
        this->etat = "en vol";
    }
    std::lock_guard<std::mutex> lock(mutexDeviation);
    this->enDeviation = false;
    this->aAtteintPointDeviation = false;
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

void Avion::consommerCarburant(std::string etat) {
    double facteur = tempsRef.getFacteurTemps();
    
    if(this->etat == "en vol" || this->etat == "en attente") {
        this->carburant -= this->consommation * facteur;
    } else if(this->etat == "decollage"){
        this->carburant -= 1 * facteur;
    } else if(this->etat == "atterrissage") {
        this->carburant -= 0.5 * facteur;
    }

    if (carburant < 0) {
        this->carburant = 0;
    }
}

void Avion::atterrissage() {
    if (this->etat != "atterrissage" && this->etat != "au sol") {
        this->etat = "atterrissage";
        this->enDeplacement = false;
        std::lock_guard<std::mutex> lock(mutexDeviation);
        this->enDeviation = false;
        this->aAtteintPointDeviation = false;
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

sf::Angle Avion::inclinaison() {
    double dx, dy;
    double angleCible;
    
    if (enDeviation) {
        if (!aAtteintPointDeviation) {
            dx = pointDeviationX - positionX;
            dy = pointDeviationY - positionY;
        } else {
            dx = destinationOriginaleX - positionX;
            dy = destinationOriginaleY - positionY;
        }
    } else {
        dx = destinationX - positionX;
        dy = destinationY - positionY;
    }
    
    double distance = sqrt(dx * dx + dy * dy);
    
    if (distance < 1.0) {
        return this->angle;
    }
    
    double rad = std::atan2(dy, dx);
    angleCible = rad * (180.0 / M_PI);
    
    angleCible += 90.0;
    
    if (angleCible < 0) {
        angleCible += 360.0;
    } else if (angleCible >= 360.0) {
        angleCible -= 360.0;
    }
    
    double angleActuel = this->angle.asDegrees();
    
    if (angleActuel < 0) {
        angleActuel += 360.0;
    } else if (angleActuel >= 360.0) {
        angleActuel -= 360.0;
    }
    
    double diff = angleCible - angleActuel;
    
    if (diff > 180.0) {
        diff -= 360.0;
    } else if (diff < -180.0) {
        diff += 360.0;
    }
    
    double facteurInterpolation = 0.01; 
    
    if (enDeviation) {
        facteurInterpolation = 0.05; 
    } else if (etat == "atterrissage") {
        facteurInterpolation = 0.05; 
    } else if (etat == "decollage") {
        facteurInterpolation = 0.08; 
    } else if (etat == "en attente") {
        facteurInterpolation = 0.03;
    }
    
    facteurInterpolation *= tempsRef.getFacteurTemps();
    
    if (facteurInterpolation > 0.5) {
        facteurInterpolation = 0.5;
    }
    
    double nouvelAngle = angleActuel + (diff * facteurInterpolation);
    
    if (nouvelAngle < 0) {
        nouvelAngle += 360.0;
    } else if (nouvelAngle >= 360.0) {
        nouvelAngle -= 360.0;
    }
    
    this->angle = sf::degrees(nouvelAngle);
    return this->angle;
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

std::vector<std::string> Aeroport::getParking() {
    return this->parking;
}

void Aeroport::setParking(int indice, std::string etat) {
    if (indice >= 0 && indice < parking.size()) {
        this->parking[indice] = etat;
    }
}

bool Aeroport::parkingvide() {
    int compteur = 0;
    for(int i = 0; i < this->parking.size(); i++) {
        if(parking[i] != "Rien") {
            compteur++;
        }
    }
    if(compteur == this->parking.size()) {
        return false;
    } else {
        return true;
    }
}

bool TourControle::gererGarer(std::unique_ptr<Avion>& avion) {
    if (avion->estgare) {
        return false;
    }
    
    std::vector<std::string> le_parking = avion->destination->getParking();
    
    for(int i = 0; i < le_parking.size(); i++) {
        if(le_parking[i] == "Rien") {
            avion->destination->setParking(i, avion->getId());
            avion->estgare = true;
            return true; 
        }
    }
    
    return false;
}

Monde::Monde() {}

void Monde::initialiser() {
}

void Monde::demarrerSimulation() {
}

void Monde::arreterSimulation() {
}

void Monde::ajouterAvion(std::unique_ptr<Avion> avion) {
    avions.push_back(std::move(avion));
}

Journal::Journal(const std::string& nomFichier) {
    fichier.open("../Code/" + nomFichier);
}

void Journal::log(const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex);
    if (fichier.is_open()) {
        fichier << std::setw(3) << message;
    }
}

Journal::~Journal() {
    if (fichier.is_open()) {
        fichier.close();
    }
}

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

void Temps::setHeure(int monheure) {
    this->heure = monheure;
}

void Temps::setMinute(int monminute) {
    this->minute = monminute;
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

void Temps::update() {
    std::lock_guard<std::mutex> lock(mutex);
    
    auto maintenant = std::chrono::steady_clock::now();
    auto delta = std::chrono::duration<float>(maintenant - dernierUpdate).count();
    dernierUpdate = maintenant;
    
    this->tempsAccumule += delta * this->facteurTemps;
    
    const float temps_simule = 1.0f; 
    
    while (tempsAccumule >= temps_simule) {
        tempsAccumule -= temps_simule;
        this->minute++;
        
        if (this->minute >= 60) {
            this->minute = 0;
            this->heure++;
            
            if (this->heure >= 24) {
                this->heure = 0;
            }
        }
    }
}

Simulation::Simulation() : path_image("../Pictures/") {}

void Simulation::executer() {
    Monde monde;
    bool pouravion;
    monde.initialiser();
    monde.demarrerSimulation();
    const sf::Vector2u WINDOW_SIZE(1300, 805);
    sf::RenderWindow app(sf::VideoMode({WINDOW_SIZE.x, WINDOW_SIZE.y}, 32), "Projet_INFO");
    app.setFramerateLimit(60);

    sf::Texture backgroundImage, avionTexture, aeroportTexture, aeroportTexturelibre, aeroportTexturepaslibre, avionTexturemarche;
    sf::Font font;

    if (!backgroundImage.loadFromFile(path_image + "background.png") || 
        !avionTexture.loadFromFile(path_image + "avion.png") || 
        !aeroportTexture.loadFromFile(path_image + "aeroport.png") || !aeroportTexturelibre.loadFromFile(path_image + "aeroportlibre.png") || !font.openFromFile(path_image + "arial.ttf") || !aeroportTexturepaslibre.loadFromFile(path_image + "aeroportpaslibre.png")) {
        throw std::runtime_error("Erreur pendant le chargement des images");
    }

    sf::Sprite backgroundSprite(backgroundImage), avionSprite(avionTexture), avionSprite2(avionTexture);
    std::vector<sf::Sprite> aeroportsSprite;
    std::vector<sf::Sprite> avionsSprite;
    avionsSprite.push_back(avionSprite);
    avionsSprite.push_back(avionSprite2);

    CentreControle centre(&monde);
    std::vector<Aeroport>& aeroports = centre.tous_les_aeroports;
    std::vector<TourControle>& tour_de_controles = centre.tous_les_tours_de_controles;

    centre.tous_les_aeroports[5].setParking(0, "Pris");
    auto& avions = centre.tous_les_avions;

    Aeroport aeroportDepart = aeroports[0];
    Aeroport aeroportArrivee = aeroports[8];
    for(int i = 0;i<avionsSprite.size();i++) {
        avionsSprite[i].setScale(sf::Vector2f(0.4, 0.4));
    }    
    
    avions[0]->start();
    avions[0]->decollage();
    avions[0]->setDestination(aeroports[8]);
    
    avions[1]->start();
    avions[1]->decollage();
    avions[1]->setDestination(aeroports[0]);
    
    int counter = 0;
    Journal journal("monlog.txt");

    sf::Text texteFacteurTemps(font);
    sf::Text horloge(font);
    texteFacteurTemps.setFont(font);
    texteFacteurTemps.setCharacterSize(20);
    texteFacteurTemps.setFillColor(sf::Color::White);
    texteFacteurTemps.setPosition(sf::Vector2f(static_cast<float>(1080), static_cast<float>(60)));
    horloge.setFont(font);
    horloge.setCharacterSize(50);
    horloge.setFillColor(sf::Color::White);
    horloge.setPosition(sf::Vector2f(static_cast<float>(1090), static_cast<float>(0)));
    
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
        monde.getTemps().update();
        texteFacteurTemps.setString("Vitesse du temps: " + std::to_string(monde.getTemps().getFacteurTemps()).substr(0, 3) + "x");
        std::string heureStr = (monde.getTemps().getHeure() < 10 ? "0" : "") + std::to_string(monde.getTemps().getHeure());
        std::string minuteStr = (monde.getTemps().getMinute() < 10 ? "0" : "") + std::to_string(monde.getTemps().getMinute());
        horloge.setString(heureStr + " : " + minuteStr + "H");

        for (size_t i = 0; i < avions.size(); i++) {
            if (!avions[i]->volDemarre) {
                if (i == 0) {
                    avionsSprite[0].setRotation(avions[i]->inclinaison());
                } else {
                    avionsSprite[1].setRotation(avions[i]->inclinaison());
                }
                
                avions[i]->volDemarre = true;
                
                if (i == 0) {
                    std::cout << "Avion " << avions[i]->getId() << " décollage de " << aeroportDepart.getId() 
                              << " vers " << aeroportArrivee.getId() << std::endl;
                } else {
                    std::cout << "Avion " << avions[i]->getId() << " décollage de " << aeroportArrivee.getId() 
                              << " vers " << aeroportDepart.getId() << std::endl;
                }
            }
            
            for (size_t j = i + 1; j < avions.size(); j++) {
                if (avions[i]->verifierRisqueCollision(avions[j].get())) {
                    avions[i]->calculerDeviation(avions[j].get());
                    avions[j]->calculerDeviation(avions[i].get());
                }
            }
        }

        for (int i = 0; i < 2; ++i) {
            Avion* a = avions[i].get();

            if (a->getEtat() == "au sol")
                continue;

            avionsSprite[i].setPosition({ 
                static_cast<float>(a->getPositionX()),
                static_cast<float>(a->getPositionY())
            });

            avionsSprite[i].setRotation(a->inclinaison());
        }
        
        for (size_t i = 0; i < aeroports.size(); i++) {
            if(aeroports[i].parkingvide()) {
                sf::Sprite aeroportSprite(aeroportTexturelibre);
                aeroportSprite.setScale(sf::Vector2f(0.12, 0.12));
                aeroportSprite.setPosition(sf::Vector2f(aeroports[i].getPositionX(), aeroports[i].getPositionY()));
                aeroportsSprite.push_back(aeroportSprite);
            } else {
                sf::Sprite aeroportSprite(aeroportTexturepaslibre);
                aeroportSprite.setScale(sf::Vector2f(0.12, 0.12));
                aeroportSprite.setPosition(sf::Vector2f(aeroports[i].getPositionX(), aeroports[i].getPositionY()));
                aeroportsSprite.push_back(aeroportSprite);
            }
        }

        if (counter++ % 60 == 0) {
            for (size_t i = 0; i < avions.size(); i++) {
                if(!avions[i]->estBienAuSol()) {
                    std::cout << "Avion " << avions[i]->getId() << " - Position: (" 
                              << std::setfill(' ')<< std::setw(7.0)<< avions[i]->getPositionX() << ", " 
                              << std::setfill(' ')<< std::setw(7.0)<< avions[i]->getPositionY() << ", " 
                              << std::setfill(' ')<< std::setw(7.0)<< avions[i]->getPositionZ() << ")" 
                              << " - carburant: " << std::setfill(' ')<<std::setw(4.0)<< avions[i]->getCarburant() 
                              << " - état: " << avions[i]->getEtat() << std::endl;


                    journal.log("Avion");
                    journal.log(avions[i]->getId());
                    journal.log(" - Position: ");
                    journal.log(std::to_string((int)avions[i]->getPositionX())+",");
                    journal.log(std::to_string((int)avions[i]->getPositionY())+",");
                    journal.log(std::to_string((int)avions[i]->getPositionZ()));
                    journal.log(" - carburant: ");
                    journal.log(std::to_string((int)avions[i]->getCarburant()));
                    journal.log(" - etat: ");
                    journal.log(avions[i]->getEtat());
                    journal.log("\n");
                }
            }
        }

    if(counter > 800) {
        centre.tous_les_aeroports[0].setParking(0, "Rien");
    }


        for(int i = 0;i<avions.size();i++) {
            if(avions[i]->getEtat() == "au sol" && !avions[i]->estgare) {
                bool resultat = tour_de_controles[i].gererGarer(avions[i]);
                if(resultat) {
                    std::cout << "good" << std::endl;
                    avions[i]->setBienAuSol();
                    avions[i]->estgare = true;
                    avionsSprite[i].setPosition(sf::Vector2f(10000.0, 10000.0));
                } else {
                    if(avions[i]->getEtat() != "en attente") {
                        avions[i]->setEtat("en attente");
                    }
                }
            }
        }

        app.clear();
        app.draw(backgroundSprite);
        app.draw(avionsSprite[0]);
        app.draw(avionsSprite[1]);
        
        for (auto& aeroportSprite : aeroportsSprite) {
            app.draw(aeroportSprite);
        }
        
        if (font.getInfo().family != "") {
            app.draw(texteFacteurTemps);
            app.draw(horloge);
        }
        
        app.display();
        aeroportsSprite.clear();
    }

    for (auto& avion : avions) {
        avion->stop();
    }
    monde.arreterSimulation();
}
