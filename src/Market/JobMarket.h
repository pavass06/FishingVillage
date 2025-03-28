#ifndef JOBMARKET_H
#define JOBMARKET_H

#include "Market.h"
#include <vector>
#include <string>
#include <memory>
#include <random>
#include <iostream>

// Forward declarations to avoid circular includes
class FisherMan;
class FishingFirm;

// Struct representing a job posting from a firm
struct JobPosting {
    int firmID;
    std::string jobSector;
    int educationRequirement;
    int experienceRequirement;
    int attractiveness;
    int vacancies;
    bool recruiting;
};

// Struct representing a job application from a worker
struct JobApplication {
    int workerID;
    std::string desiredSector;
    int educationLevel;
    int experienceLevel;
    int preference;
    int quantity;
    bool matched;
    std::shared_ptr<FisherMan> fisherman;
};

class JobMarket : public Market {
private:
    std::vector<JobPosting> postings;
    std::vector<JobApplication> applications;
    int matchedJobs;
    double meanFishOrder;
    double currentFishPrice;

    // Pointer to external list of firms to facilitate hiring
    std::vector<std::shared_ptr<FishingFirm>>* firmList;

public:
    JobMarket(double initWage, double fishPrice, double meanOrder = 1.5);

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
