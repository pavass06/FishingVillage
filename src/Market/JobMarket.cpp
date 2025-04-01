#include "JobMarket.h"
#include "FishingFirm.h"   // Full definition needed to call getID(), addEmployee()
#include "FisherMan.h"     // Full definition needed if accessing FisherMan methods

#include <iostream>
#include <algorithm>
#include <random>

JobMarket::JobMarket(double initWage, double fishPrice, double meanOrder)
    : Market(initWage),
      matchedJobs(0),
      meanFishOrder(meanOrder),
      currentFishPrice(fishPrice),
      firmList(nullptr)
{
    clearingPrice = currentFishPrice * meanFishOrder;
}

void JobMarket::setCurrentFishPrice(double fishPrice) {
    currentFishPrice = fishPrice;
}

void JobMarket::setFirmList(std::vector<std::shared_ptr<FishingFirm>>* firms) {
    firmList = firms;
}

void JobMarket::submitJobPosting(const JobPosting &posting) {
    postings.push_back(posting);
    aggregateSupply += posting.vacancies;
}

void JobMarket::submitJobApplication(const JobApplication &application) {
    JobApplication app = application;
    app.matched = false;
    applications.push_back(app);
    aggregateDemand += app.quantity;
}

void JobMarket::clearMarket(std::default_random_engine &generator) {
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

void JobMarket::reset() {
    postings.clear();
    applications.clear();
    aggregateDemand = 0;
    aggregateSupply = 0;
    matchedJobs = 0;
}

void JobMarket::print() const {
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

int JobMarket::getMatchedJobs() const {
    return matchedJobs;
}

JobMarket::~JobMarket() = default;



