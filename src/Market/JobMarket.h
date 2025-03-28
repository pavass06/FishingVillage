#ifndef JOBMARKET_H
#define JOBMARKET_H

#include "Market.h"
#include "FisherMan.h"    // Pour avoir la définition de FisherMan
#include "FishingFirm.h"  // Pour avoir la définition de FishingFirm
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <random>
#include <memory>

struct JobPosting {
    int firmID;
    std::string jobSector;
    int educationRequirement;
    int experienceRequirement;
    int attractiveness;
    int vacancies;
    bool recruiting;
};

struct JobApplication {
    int workerID;
    std::string desiredSector;
    int educationLevel;
    int experienceLevel;
    int preference;
    int quantity;
    bool matched;
    // Ce champ permet à JobMarket de faire le lien entre la candidature et le FisherMan postulant.
    std::shared_ptr<FisherMan> fisherman;
};

class JobMarket : public Market {
private:
    std::vector<JobPosting> postings;
    std::vector<JobApplication> applications;
    int matchedJobs;
    double meanFishOrder;    // Consommation moyenne de poisson par personne.
    double currentFishPrice; // Prix courant du poisson.
    // Pointeur vers la liste des firmes pour embaucher directement.
    std::vector<std::shared_ptr<FishingFirm>>* firmList;
public:
    JobMarket(double initWage, double fishPrice, double meanOrder = 1.5)
        : Market(initWage), matchedJobs(0), meanFishOrder(meanOrder), currentFishPrice(fishPrice), firmList(nullptr)
    {
        clearingPrice = currentFishPrice * meanFishOrder;
    }

    virtual ~JobMarket() {}

    double getClearingWage() const { return clearingPrice; }

    // Permet de mettre à jour le prix du poisson.
    void setCurrentFishPrice(double fishPrice) {
        currentFishPrice = fishPrice;
    }

    // Permet d'assigner la liste des firmes afin que JobMarket puisse y accéder pour l'embauche.
    void setFirmList(std::vector<std::shared_ptr<FishingFirm>>* firms) {
        firmList = firms;
    }

    void submitJobPosting(const JobPosting &posting) {
        postings.push_back(posting);
        aggregateSupply += posting.vacancies;
    }

    void submitJobApplication(const JobApplication &application) {
        JobApplication app = application;
        app.matched = false;
        applications.push_back(app);
        aggregateDemand += app.quantity;
    }

    // Le processus de matching : pour chaque offre, on cherche une candidature dont le secteur correspond.
    // Lorsque la correspondance est trouvée, JobMarket utilise firmList pour identifier la firme correspondante
    // et appelle addEmployee() pour y ajouter le FisherMan postulant.
    virtual void clearMarket(std::default_random_engine &generator) override {
        matchedJobs = 0;
        for (auto &posting : postings) {
            for (auto &app : applications) {
                if (!app.matched) {
                    if (posting.jobSector == app.desiredSector && posting.recruiting) {
                        posting.vacancies -= 1;
                        app.matched = true;
                        matchedJobs++;
                        if (firmList) {
                            for (auto &firm : *firmList) {
                                if (firm->getID() == posting.firmID) {
                                    firm->addEmployee(app.fisherman);
                                    break;
                                }
                            }
                        }
                        if (posting.vacancies <= 0) {
                            posting.recruiting = false;
                            break;
                        }
                    }
                }
            }
        }
        clearingPrice = currentFishPrice * meanFishOrder;
    }

    virtual void reset() override {
        postings.clear();
        applications.clear();
        aggregateDemand = 0;
        aggregateSupply = 0;
        matchedJobs = 0;
    }

    virtual void print() const override {
#if verbose==1
        std::cout << "-----------" << std::endl;
        std::cout << "JobMarket State:" << std::endl;
        std::cout << "Aggregate Demand (Applications): " << aggregateDemand << std::endl;
        std::cout << "Aggregate Supply (Vacancies): " << aggregateSupply << std::endl;
        std::cout << "Clearing Wage (Based on fish price): " << clearingPrice << std::endl;
        std::cout << "Matched Jobs: " << matchedJobs << std::endl;
        std::cout << "Total Postings: " << postings.size() 
                  << " | Total Applications: " << applications.size() << std::endl;
#endif
    }

    int getMatchedJobs() const { return matchedJobs; }
};

#endif // JOBMARKET_H
