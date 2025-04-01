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
    // Au lieu d'un booléen "employed", nous utilisons firmID.
    // 0 signifie non employé, toute autre valeur correspond à l'ID de la firme employeuse.
    int firmID;
    double wage;  // Salaire journalier lorsque employé

    // Attributs spécifiques au travail :
    std::string jobSector;  // Dans notre village, c'est "fishing"
    int educationLevel;     // Niveau de compétence en pêche (1-5)
    int experienceLevel;    // Niveau d'expérience (1-5)
    int jobPreference;      // Attractivité minimale acceptable (1-5)

public:
    // Constructeur avec un paramètre supplémentaire firmID (0 si non employé)
    FisherMan(int id, double initFunds, int lifetime, double income, double savings,
              double jobDemand, double goodsDemand, int firmID, double wage,
              double unemploymentBenefit, 
              const std::string &jobSector, int educationLevel, int experienceLevel, int jobPreference)
        : Household(id, initFunds, lifetime, income, savings, jobDemand, goodsDemand),
          firmID(firmID), wage(wage),
          jobSector(jobSector), educationLevel(educationLevel),
          experienceLevel(experienceLevel), jobPreference(jobPreference)
    {}

    virtual ~FisherMan() {}

    // Si le pêcheur est employé (firmID != 0), il reçoit son salaire.
    virtual void act() override {
        if (firmID != 0) {
            funds += wage;
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
                  << " | Job Preference: " << jobPreference << std::endl;
#endif
    }

    // Getters et setters pour les attributs spécifiques.
    std::string getJobSector() const { return jobSector; }
    void setJobSector(const std::string &js) { jobSector = js; }
    
    double getAge() const { return age; }

    int getEducationLevel() const { return educationLevel; }
    void setEducationLevel(int el) { educationLevel = el; }

    int getExperienceLevel() const { return experienceLevel; }
    void setExperienceLevel(int exp) { experienceLevel = exp; }

    int getJobPreference() const { return jobPreference; }
    void setJobPreference(int jp) { jobPreference = jp; }

    // Nouveaux getters et setters pour firmID.
    int getFirmID() const { return firmID; }
    void setFirmID(int id) { firmID = id; }

    double getWage() const { return wage; }
    void setWage(double w) { wage = w; }

    // La candidature ne doit être générée que si le pêcheur est au chômage (firmID == 0).
    JobApplication generateJobApplication() const;

};

#endif // FISHERMAN_H
