#include<iostream>
#include<thread>

class Agent {
protected:
    std::string id;
    std::thread thread_;
    bool actif;

public:
    Agent(const std::string& id);
    virtual ~Agent();
    virtual void run() = 0;
    void start();
    void stop();

    std::string getId() const;
};