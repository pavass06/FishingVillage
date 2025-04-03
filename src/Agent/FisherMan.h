#ifndef FISHERMAN_H
#define FISHERMAN_H

#include "Household.h"
//#include "JobMarket.h"  // Pour la structure JobApplication
struct JobApplication;  // <--- Forward declaration
#include <iostream>
#include <string>

class FishingFirm;

class FisherMan : public Household {
protected:
    // Instead of a boolean "employed", we use firmID.
    // 0 means unemployed, any other value corresponds to the employer firm's ID.
    int firmID;
    double wage;  // Daily wage when employed

    // Job-specific attributes:
    std::string jobSector;  // In our village, it's "fishing"
    int educationLevel;     // Fishing skill level (1-5)
    int experienceLevel;    // Experience level (1-5)
    int jobPreference;      // Minimum acceptable attractiveness (1-5)

    // NEW: Flag indicating if the fisherman is actively looking for a job.
    bool looking_for_job;

public:
    // Constructor with firmID parameter (0 if unemployed). Initialize looking_for_job accordingly.
    FisherMan(int id, double initFunds, int lifetime, double income, double savings,
              double jobDemand, double goodsDemand, int firmID, double wage,
              double unemploymentBenefit, 
              const std::string &jobSector, int educationLevel, int experienceLevel, int jobPreference)
        : Household(id, initFunds, lifetime, income, savings, jobDemand, goodsDemand),
          firmID(firmID), wage(wage),
          jobSector(jobSector), educationLevel(educationLevel),
          experienceLevel(experienceLevel), jobPreference(jobPreference)
    {
        // If unemployed, mark as looking for a job.
        looking_for_job = (firmID == 0);
    }

    virtual ~FisherMan() {}

    // If employed (firmID != 0), he receives his wage.
    virtual void act() override {
        if (firmID != 0) {
            funds += wage;
            // When working, he is not actively looking for a new job.
            looking_for_job = false;
        }
    }

    virtual void print() const override {
        Household::print();
#if verbose
        std::cout << "FirmID: " << firmID
                  << " | Wage: " << wage
                  << " | Job Sector: " << jobSector
                  << " | Fishing Skill (Edu Level): " << educationLevel 
                  << " | Experience Level: " << experienceLevel 
                  << " | Job Preference: " << jobPreference 
                  << " | Looking for job: " << (looking_for_job ? "Yes" : "No") << std::endl;
#endif
    }

    // Getters and setters for specific attributes.
    std::string getJobSector() const { return jobSector; }
    void setJobSector(const std::string &js) { jobSector = js; }
    
    double getAge() const { return age; }

    int getEducationLevel() const { return educationLevel; }
    void setEducationLevel(int el) { educationLevel = el; }

    int getExperienceLevel() const { return experienceLevel; }
    void setExperienceLevel(int exp) { experienceLevel = exp; }

    int getJobPreference() const { return jobPreference; }
    void setJobPreference(int jp) { jobPreference = jp; }

    // Getters and setters for firmID.
    int getFirmID() const { return firmID; }
    void setFirmID(int id) { 
        firmID = id; 
        // When hired (firmID != 0), stop job search; if 0, start job search.
        looking_for_job = (id == 0);
    }

    double getWage() const { return wage; }
    void setWage(double w) { wage = w; }

    // NEW: Getter for looking_for_job flag.
    bool isLookingForJob() const { return looking_for_job; }
    void setLookingForJob(bool flag) { looking_for_job = flag; }

    // A job application should only be generated if the fisherman is unemployed.
    // We add a check to only generate if looking_for_job is true.
    JobApplication generateJobApplication() const;
};

#endif // FISHERMAN_H
