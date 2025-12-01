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

void Avion::setDestination(Aeroport & aeroport) {
    this->destinationX = aeroport.getPositionX() + 30;
    this->destinationY = aeroport.getPositionY() + 30;
    this->enDeplacement = true;
    this->enApprocheFinale = false; 
    this->destination = &aeroport;
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
// test 
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

std::vector<std::string> Aeroport::getParking() {
    return this->parking;
}

void Aeroport::setParking(int indice, std::string etat) {
    this->parking[indice] = etat;
}

bool Aeroport::parkingvide() {
    int compteur = 0;
    for(int i = 0;i<this->parking.size();i++) {
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







void TourControle::gererGarer(std::unique_ptr<Avion>& avion) {
    std::vector<std::string> le_parking = avion->destination->getParking();
    for(int i = 0;i<le_parking.size();i++) {
        if(le_parking[i] == "Rien" || !avion->estgare) {
            avion->destination->setParking(i, avion->getId());
            avion->estgare = true;
        }
    }
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
        fichier << message << std::endl;
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
    monde.initialiser();
    monde.demarrerSimulation();
    const sf::Vector2u WINDOW_SIZE(1300, 805);
    sf::RenderWindow app(sf::VideoMode({WINDOW_SIZE.x, WINDOW_SIZE.y}, 32), "Projet_INFO");
    app.setFramerateLimit(60);

    sf::Texture backgroundImage, avionTexture, aeroportTexture, aeroportTexturelibre, aeroportTexturepaslibre;
    sf::Font font;

    if (!backgroundImage.loadFromFile(path_image + "background.png") || 
        !avionTexture.loadFromFile(path_image + "avion.png") || 
        !aeroportTexture.loadFromFile(path_image + "aeroport.png") || !aeroportTexturelibre.loadFromFile(path_image + "aeroportlibre.png") || !font.openFromFile(path_image + "arial.ttf") || !aeroportTexturepaslibre.loadFromFile(path_image + "aeroportpaslibre.png")) {
        throw std::runtime_error("Erreur pendant le chargement des images");
    }

    sf::Sprite backgroundSprite(backgroundImage), avionSprite(avionTexture);
    std::vector<sf::Sprite> aeroportsSprite;

    CentreControle centre(&monde);
    std::vector<Aeroport>& aeroports = centre.tous_les_aeroports;  // Notez le '&' pour une référence
    std::vector<TourControle>& tour_de_controles = centre.tous_les_tours_de_controles;                                                                   //
                                                                   //
    centre.tous_les_aeroports[5].setParking(0, "Pris");
    auto& avions = centre.tous_les_avions;
    


    Aeroport aeroportDepart = aeroports[0];
    avionSprite.setScale(sf::Vector2f(0.6, 0.6));
    
    avions[0]->start();
    avions[0]->decollage();
    avions[0]->setDestination(aeroports[8]);
    int counter = 0;
    Journal journal("monlog.txt");

    
    sf::Text texteFacteurTemps(font);
    sf::Text horloge(font);
    texteFacteurTemps.setFont(font);
    texteFacteurTemps.setCharacterSize(30);
    texteFacteurTemps.setFillColor(sf::Color::White);
    texteFacteurTemps.setPosition(sf::Vector2f(static_cast<float>(10), static_cast<float>(0)));

    horloge.setFont(font);
    horloge.setCharacterSize(50);
    horloge.setFillColor(sf::Color::White);
    horloge.setPosition(sf::Vector2f(static_cast<float>(1100), static_cast<float>(10)));
    
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
        horloge.setString(heureStr + ":" + minuteStr + "H");

        if (!avions[0]->volDemarre) {
            avionSprite.setRotation(avions[0]->inclinaison());
            if (avions[0]->volDemarre && avions[0]->getEtat() == "en vol" && 
                sqrt(pow(avions[0]->getPositionX() - aeroportDepart.getPositionX(), 2) + 
                     pow(avions[0]->getPositionY() - aeroportDepart.getPositionY(), 2)) < 50) {
                avions[0]->atterrissage();
            }
            avions[0]->volDemarre = true;
            std::cout << "Position avion: (" << avions[0]->getPositionX() << ", " 
                      << avions[0]->getPositionY() << ", " << avions[0]->getPositionZ() << ")" 
                      << " - carburant: " << avions[0]->getCarburant() 
                      << " - état: " << "vol vers " << aeroportDepart.getId() << std::endl;
        }

        if(avions[0]->getEtat() != "au sol") {
            avionSprite.setPosition(sf::Vector2f(static_cast<float>(avions[0]->getPositionX()), 
                                                static_cast<float>(avions[0]->getPositionY())));
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
            if(!avions[0]->estBienAuSol()) {
                std::cout << "Il est " 
                          << std::setw(2) << std::setfill('0') << monde.getTemps().getHeure() 
                          << ":" 
                          << std::setw(2) << std::setfill('0') << monde.getTemps().getMinute() 
                          << "H - Position avion: (" 
                          << avions[0]->getPositionX() << ", " 
                          << avions[0]->getPositionY() << ", " 
                          << avions[0]->getPositionZ() << ")" 
                          << " - carburant: " << avions[0]->getCarburant() 
                          << " - état: " << avions[0]->getEtat() << std::endl;

                journal.log("Position avion:" + std::to_string(avions[0]->getPositionX()) + "," + 
                   std::to_string(avions[0]->getPositionY()) + "," + 
                   std::to_string(avions[0]->getPositionZ()) + 
                   " - carburant:" + std::to_string(avions[0]->getCarburant()) +
                   " - état:" + avions[0]->getEtat());

                if(avions[0]->getEtat() == "au sol") {
                    avions[0]->setBienAuSol();
                    avionSprite.setPosition(sf::Vector2f(static_cast<float>(10000), static_cast<float>(10000)));
                    tour_de_controles[0].gererGarer(avions[0]);
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
            app.draw(horloge);
        }
        
        app.display();
    }

    avions[0]->stop();
    monde.arreterSimulation();
}
