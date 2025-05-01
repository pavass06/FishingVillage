#ifndef JOBMARKET_H
#define JOBMARKET_H

#include "Market.h"
#include "/Users/avass/Documents/1SSE/Code/FishingVillage/src/SimulationParameters.h"    // ← ajouté pour accéder aux params
#include "LabourModel.h"                // ← ajouté pour computeJobPostings
#include "JobTypes.h" 
#include <vector>
#include <string>
#include <memory>
#include <random>
#include <iostream>

class FishingFirm;  // Déclaration anticipée

class JobMarket : public Market {
private:
    std::vector<JobPosting> postings;
    std::vector<JobApplication> applications;
    int matchedJobs;
    double meanFishOrder;
    double currentFishPrice;

    // Pointeur vers la liste des firmes pour pouvoir embaucher
    std::vector<std::shared_ptr<FishingFirm>>* firmList;

    // Référence aux paramètres de simulation
    const SimulationParameters& params;

public:
    // Constructeur : on passe params en premier
    JobMarket(const SimulationParameters& params,
              double initWage,
              double fishPrice,
              double meanOrder = 1.5);

    virtual ~JobMarket();

    void setCurrentFishPrice(double fishPrice);
    void setFirmList(std::vector<std::shared_ptr<FishingFirm>>* firms);

    void submitJobPosting(const JobPosting& posting);
    void submitJobApplication(const JobApplication& application);

    virtual void clearMarket(std::default_random_engine& generator) override;
    virtual void reset() override;
    virtual void print() const override;

    double getClearingWage() const;
    int getMatchedJobs() const;
};

#endif // JOBMARKET_H
