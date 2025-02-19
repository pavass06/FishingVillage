#ifndef FISHERMAN_H
#define FISHERMAN_H

#include "Household.h"
#include "JobMarket.h"  // For the JobApplication struct
#include <iostream>
#include <string>

class FisherMan : public Household {
protected:
    bool employed;               // Employment status in the fishing industry
    double wage;                 // Daily wage when employed (set later via job matching)
    // In our village model, unemployed fishermen receive no income
    // (we omit unemploymentBenefit here for simplicity)
    
    // Job-specific attributes:
    std::string jobSector;       // For our village, this is fixed to "fishing"
    int educationLevel;          // Fishing skill level (a single numeric value, 1-5)
                               // Note: In this model, this represents a unique skill measure.
    int experienceLevel;         // Experience level in fishing (a single number, 1-5)
                               // This value is intended to evolve in future models.
    int jobPreference;           // Minimum acceptable job attractiveness (1-5)
                               // This single number represents the worker's preference threshold.

public:
    FisherMan(int id, double initFunds, int lifetime, double income, double savings,
              double jobDemand, double goodsDemand, bool employed, double wage,
              double unemploymentBenefit, 
              const std::string &jobSector, int educationLevel, int experienceLevel, int jobPreference)
        : Household(id, initFunds, lifetime, income, savings, jobDemand, goodsDemand),
          employed(employed), wage(wage),
          jobSector(jobSector), educationLevel(educationLevel),
          experienceLevel(experienceLevel), jobPreference(jobPreference)
    {}

    virtual ~FisherMan() {}

    // act(): If employed, the fisherman receives his wage.
    // Unemployed fishermen receive no income.
    virtual void act() override {
        if (employed) {
            funds += wage;
        }
    }

    virtual void print() const override {
        Household::print();
        std::cout << "Employment: " << (employed ? "Employed" : "Unemployed")
                  << " | Wage: " << wage
                  << " | Job Sector: " << jobSector
                  << " | Fishing Skill (Edu Level): " << educationLevel 
                  << " | Experience Level: " << experienceLevel 
                  << " | Job Preference: " << jobPreference << std::endl;
    }

    // Getters and Setters for job-specific attributes:
    std::string getJobSector() const { return jobSector; }
    void setJobSector(const std::string &js) { jobSector = js; }

    int getEducationLevel() const { return educationLevel; }
    void setEducationLevel(int el) { educationLevel = el; }

    int getExperienceLevel() const { return experienceLevel; }
    void setExperienceLevel(int exp) { experienceLevel = exp; }

    int getJobPreference() const { return jobPreference; }
    void setJobPreference(int jp) { jobPreference = jp; }

    // generateJobApplication(): Creates a unique JobApplication for this fisherman.
    // Fishermen submit one application per cycle when unemployed.
    virtual JobApplication generateJobApplication() const {
        JobApplication app;
        app.workerID = getID();
        app.desiredSector = jobSector;  // Typically "fishing"
        app.educationLevel = educationLevel;
        app.experienceLevel = experienceLevel;
        app.preference = jobPreference;
        app.quantity = 1;  // One application per cycle
        app.matched = false;
        return app;
    }

    // Getters and setters for employment status and wage:
    bool isEmployed() const { return employed; }
    void setEmployed(bool e) { employed = e; }

    double getWage() const { return wage; }
    void setWage(double w) { wage = w; }
};

#endif // FISHERMAN_H
