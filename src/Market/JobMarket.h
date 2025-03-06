#ifndef JOBMARKET_H
#define JOBMARKET_H

#include "Market.h"
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <random>

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
};

class JobMarket : public Market {
private:
    std::vector<JobPosting> postings;
    std::vector<JobApplication> applications;
    int matchedJobs;
    // New parameters for wage determination based on fish price
    double meanFishOrder;    // Average fish consumption per person (e.g., 1.5)
    double currentFishPrice; // Current price of a fish (e.g., starts at 5)
public:
    // Constructor now initializes the wage based on fish price and mean fish order.
    // initWage is provided but will be overridden by our fish-based wage calculation.
    JobMarket(double initWage, double fishPrice, double meanOrder = 1.5)
        : Market(initWage), matchedJobs(0), meanFishOrder(meanOrder), currentFishPrice(fishPrice)
    {
        // Initialize the wage based on the current fish price and consumption rate.
        clearingPrice = currentFishPrice * meanFishOrder;
    }

    virtual ~JobMarket() {}

    double getClearingWage() const { return clearingPrice; }

    // Allow external updating of the fish price (e.g., if inflation changes the price).
    void setCurrentFishPrice(double fishPrice) {
        currentFishPrice = fishPrice;
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

    // clearMarket now focuses on matching jobs and recalculating the wage based on the fish price.
    virtual void clearMarket(std::default_random_engine &generator) override {
        // Process matching between postings and applications.
        matchedJobs = 0;
        for (auto &posting : postings) {
            for (auto &app : applications) {
                if (!app.matched) {
                    if (posting.jobSector == app.desiredSector && posting.recruiting) {
                        posting.vacancies -= 1;
                        app.matched = true;
                        matchedJobs++;
                        if (posting.vacancies <= 0) {
                            posting.recruiting = false;
                            break;
                        }
                    }
                }
            }
        }
        // Update the clearing wage based on the current fish price and mean fish order.
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
        std::cout<< "-----------" << std::endl;
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
