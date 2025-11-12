class Monde {
private:
    std::vector<std::unique_ptr<Avion>> avions;
    std::unique_ptr<CentreControleRegional> ccr;
    std::unique_ptr<ControleurApproche> app;
    std::unique_ptr<TourControle> twr;

public:
    Monde();
    void initialiser();
    void demarrerSimulation();
    void arreterSimulation();
};
