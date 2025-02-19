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
public:
    JobMarket(double initPrice)
        : Market(initPrice), matchedJobs(0) {}

    virtual ~JobMarket() {}

    double getClearingWage() const { return clearingPrice; }

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

    // Override clearMarket to accept a random generator.
    virtual void clearMarket(std::default_random_engine &generator) override {
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
        if (aggregateDemand > 0) {
            double ratio = aggregateDemand / (aggregateSupply > 0 ? aggregateSupply : 1);
            if (ratio > 1.0) {
                std::normal_distribution<double> adjustDist(1.025, 0.005);
                clearingPrice *= adjustDist(generator);
            } else if (ratio < 1.0) {
                std::normal_distribution<double> adjustDist(0.975, 0.005);
                clearingPrice *= adjustDist(generator);
            }
        }
    }

    virtual void reset() override {
        postings.clear();
        applications.clear();
        aggregateDemand = 0;
        aggregateSupply = 0;
        matchedJobs = 0;
    }

    virtual void print() const override {
        std::cout << "JobMarket State:" << std::endl;
        std::cout << "Aggregate Demand (Applications): " << aggregateDemand << std::endl;
        std::cout << "Aggregate Supply (Vacancies): " << aggregateSupply << std::endl;
        std::cout << "Clearing Wage: " << clearingPrice << std::endl;
        std::cout << "Matched Jobs: " << matchedJobs << std::endl;
        std::cout << "Total Postings: " << postings.size() 
                  << " | Total Applications: " << applications.size() << std::endl;
    }

    int getMatchedJobs() const { return matchedJobs; }
};

#endif // JOBMARKET_H
