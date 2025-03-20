#ifndef WORLD_H
#define WORLD_H

#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>
#include <cstdlib>
#include <random>
#include <unordered_map>
#include "FisherMan.h"
#include "Firm.h"
#include "FishingFirm.h"
#include "JobMarket.h"
#include "FishingMarket.h"

class World {
private:
    int currentCycle;         // Current simulation day
    int totalCycles;          // Total simulation days
    double annualBirthRate;   // Annual birth rate (e.g., 0.02)
    int maxStarvingDays;      // Maximum consecutive days without food before death

    // Vectors for population and firms – World owns these
    std::vector<std::shared_ptr<FisherMan>> fishers;
    // For firms, we store pointers to FishingFirm (our derived type)
    const std::vector<std::shared_ptr<FishingFirm>>* firms; 

    std::shared_ptr<JobMarket> jobMarket;
    std::shared_ptr<FishingMarket> fishingMarket;

    double GDP;
    double unemploymentRate;
    double inflation;

    // Map: fisherID -> consecutive days without eating.
    std::unordered_map<int, int> daysWithoutEat;

public:
    // Constructor.
    World(int cycles,
          double annualBirthRate_,
          std::shared_ptr<JobMarket> jm,
          std::shared_ptr<FishingMarket> fm,
          int maxStarvingDays_)
        : currentCycle(0),
          totalCycles(cycles),
          annualBirthRate(annualBirthRate_),
          maxStarvingDays(maxStarvingDays_),
          jobMarket(jm),
          fishingMarket(fm),
          GDP(0.0),
          unemploymentRate(0.0),
          inflation(0.0),
          firms(nullptr)
    {}

    // Setter: supply pointer to the vector of FishingFirm objects.
    void setFirms(const std::vector<std::shared_ptr<FishingFirm>>& firmVec) {
         firms = &firmVec;
    }

    // Add a fisherman to the world.
    void addFisherMan(std::shared_ptr<FisherMan> f) {
        fishers.push_back(f);
        daysWithoutEat[f->getID()] = 0;
    }

    // Return total number of fishers.
    int getTotalFishers() const {
        return static_cast<int>(fishers.size());
    }

    // Return GDP.
    double getGDP() const { return GDP; }

    // Compute number of unemployed fishers.
    int getUnemployedFishers() const {
        int count = 0;
        for (const auto &fisher : fishers)
            if (!fisher->isEmployed())
                count++;
        return count;
    }

    // This function integrates the entire simulation for one day.
    // It refreshes supply by calling refreshSupply() on the fishing market,
    // then processes fishers’ actions, population growth, job matching,
    // market clearing, and starvation.
    void simulateCycle(std::default_random_engine &generator,
                       std::normal_distribution<double> &firmPriceDist,
                       std::uniform_int_distribution<int> &goodsQuantityDist,
                       std::normal_distribution<double> &consumerPriceDist) {
        // 0) Refresh supply: update each firm's stock and create new offerings.
        if (firms != nullptr) {
            fishingMarket->refreshSupply(*firms);
        } else {
            std::cerr << "Error: Firms vector not set in World." << std::endl;
        }
        
        // 1) Process FisherMen actions.
        for (auto &fisher : fishers) {
            if (fisher->isActive())
                fisher->act(); // e.g., update funds, etc.
        }
        for (auto &fisher : fishers) {
            if (fisher->isActive())
                fisher->update();
        }
        // Remove inactive fishers.
        fishers.erase(std::remove_if(fishers.begin(), fishers.end(),
            [](const std::shared_ptr<FisherMan> &f) {
                return !f->isActive();
            }), fishers.end());

        // 2) Population management: birth process.
        {
            double dailyBirthRate = annualBirthRate / 365.0;
            int currentPopulation = getTotalFishers();
            double lambda = dailyBirthRate * currentPopulation;
            std::poisson_distribution<int> poissonDist(lambda);
            int newBirths = poissonDist(generator);
            for (int i = 0; i < newBirths; i++) {
                int newID = 1000 + fishers.size();
                auto newFisher = std::make_shared<FisherMan>(
                    newID, 0.0, 365 * 60, 0.0, 0.0, 1.0, 1.0, false,
                    0.0, 0.0, "fishing", 1, 1, 1
                );
                addFisherMan(newFisher);
            }
        }

        // 3) Job Market process.
        // For each firm, submit a job posting.
        if (firms != nullptr) {
            for (auto &firm : *firms) {
                JobPosting posting = firm->generateJobPosting("fishing", 1, 1, 1);
                jobMarket->submitJobPosting(posting);
            }
        }
        // Each unemployed fisher submits a job application.
        for (auto &fisher : fishers) {
            if (!fisher->isEmployed()) {
                JobApplication app = fisher->generateJobApplication();
                jobMarket->submitJobApplication(app);
            }
        }
        jobMarket->clearMarket(generator);
        double clearingWage = jobMarket->getClearingWage();
        double dailyWage = 1.5 * clearingWage;
        int matches = jobMarket->getMatchedJobs();
        for (auto &fisher : fishers) {
            if (!fisher->isEmployed() && matches > 0) {
                fisher->setEmployed(true);
                fisher->setWage(dailyWage);
                matches--;
            }
        }
        jobMarket->reset();

        // 4) Fishing Market orders:
        // Each fisher orders 1 fish per day.
        for (auto &fisher : fishers) {
            FishOrder order;
            order.id = fisher->getID();
            order.desiredSector = "fishing";
            order.quantity = 1.0;
            order.perceivedValue = consumerPriceDist(generator);
            order.availableFunds = fisher->getFunds();
            order.hungry = (daysWithoutEat[fisher->getID()] > 0);
            fishingMarket->submitFishOrder(order);
        }
        // Force aggregate demand to equal the number of fishers.
        fishingMarket->setAggregateDemand(static_cast<double>(fishers.size()));
        fishingMarket->clearMarket(generator);
        fishingMarket->print();
        fishingMarket->reset();

        // 5) Compute daily GDP (here you need to sum up firm revenues).
        // (Assuming that firms record sales; for now, we assume GDP is computed elsewhere.)
        GDP = 0.0;
        // 6) Unemployment calculation.
        int unemployedCount = 0;
        for (const auto &fisher : fishers)
            if (!fisher->isEmployed())
                unemployedCount++;
        unemploymentRate = (fishers.size() > 0) ? static_cast<double>(unemployedCount) / fishers.size() : 0.0;

        // 7) Starvation process.
        std::unordered_map<int, double> purchases = fishingMarket->getPurchases();
        for (auto &fisher : fishers) {
            int fID = fisher->getID();
            if (purchases.find(fID) == purchases.end() || purchases[fID] < 1.0) {
                daysWithoutEat[fID]++;
            } else {
                daysWithoutEat[fID] = 0;
            }
        }
        for (auto &fisher : fishers) {
            int fID = fisher->getID();
            if (daysWithoutEat[fID] >= maxStarvingDays)
                fisher->setActive(false);
        }
        fishers.erase(std::remove_if(fishers.begin(), fishers.end(),
            [](const std::shared_ptr<FisherMan>& f) {
                return !f->isActive();
            }), fishers.end());

        currentCycle++;
    }

    void printWorldState() const {
        std::cout << "=== World State at Day " << currentCycle << " ===" << std::endl;
        std::cout << "Population: " << getTotalFishers() << std::endl;
        std::cout << "Job Market:" << std::endl;
        jobMarket->print();
        std::cout << "Fishing Market:" << std::endl;
        fishingMarket->print();
    }
};

#endif // WORLD_H
