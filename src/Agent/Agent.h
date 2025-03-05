#ifndef AGENT_H
#define AGENT_H

#include <iostream>
#include <random>

class Agent {
protected:
    int ID;               // Unique identifier
    double funds;         // Available money
    bool status;          // true = active, false = inactive
    int age;              // Current age (in cycles)
    int lifetime;         // Maximum lifespan (in cycles)
    
public:
    // Constructor: lifetime is drawn from a Gaussian distribution externally
    Agent(int id, double initFunds, int lifetime)
        : ID(id), funds(initFunds), status(true), age(0), lifetime(lifetime) {}
        
    virtual ~Agent() {}

    // Pure virtual action function: to be defined in derived classes
    virtual void act() = 0;

    virtual void setActive(bool active){
        status = active ;
    }

    // Update method: increments age and deactivates agent if age exceeds lifetime
    virtual void update() {
        age++;
        if (age >= lifetime) {
            status = false;
        }
    }

    // Print current state for debugging/monitoring
    virtual void print() const {
        std::cout << "Agent " << ID 
                  << " | Funds: " << funds 
                  << " | Age: " << age << "/" << lifetime 
                  << " | Status: " << (status ? "Active" : "Inactive") 
                  << std::endl;
    }

    // Getters
    int getID() const { return ID; }
    double getFunds() const { return funds; }
    bool isActive() const { return status; }
};

#endif // AGENT_H

