#include <iostream>

class Journal {
private:
    std::mutex mtx;
    std::ofstream fichier;

public:
    Journal(const std::string& nomFichier);
    void log(const std::string& message);
};
