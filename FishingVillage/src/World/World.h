#ifndef WORLD_H
#define WORLD_H

#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>
#include <cstdlib>
#include "FisherMan.h"    // Concrete subclass of Household.
#include "Firm.h"         // FishingFirms derive from Firm.
#include "FishingFirm.h"  // Needed so that generateGoodsOffering() is visible.
#include "JobMarket.h"
#include "FishingMarket.h"

class World {
private:
    int currentCycle;         // Each cycle represents 1 day.
    int totalCycles;          // Total number of days.
    double birthRate;         // Birth rate per day (0.1: ~1 new FisherMan every 10 days).

    std::vector<std::shared_ptr<FisherMan>> fishers;
    std::vector<std::shared_ptr<Firm>> firms;
    
    std::shared_ptr<JobMarket> jobMarket;
    std::shared_ptr<FishingMarket> fishingMarket;

    double previousFishPrice; // For inflation calculations

    double GDP;
    double unemploymentRate;  // Total people unemployed / Total FisherMen.
    double inflation;         // Percentage change in fish market clearing price.

public:
    World(int cycles, double birthRate_,
          std::shared_ptr<JobMarket> jm, std::shared_ptr<FishingMarket> fm)
        : currentCycle(0), totalCycles(cycles), birthRate(birthRate_),
          jobMarket(jm), fishingMarket(fm),
          previousFishPrice(fm->getClearingFishPrice()),
          GDP(0.0), unemploymentRate(0.0), inflation(0.0)
    {}

    const std::vector<std::shared_ptr<FisherMan>>& getFishers() const {
        return fishers;
    }

    // Returns the total number of fishermen in the world.
    int getTotalFishers() const {
        return fishers.size();
    }

// Returns the number of unemployed fishermen.
    int getUnemployedFishers() const {
    int count = 0;
        for (const auto &fisher : fishers) {
            if (!fisher->isEmployed())
                count++;
    }
    return count;
}


    void addFisherMan(std::shared_ptr<FisherMan> f) {
        fishers.push_back(f);
    }

    void addFirm(std::shared_ptr<Firm> f) {
        firms.push_back(f);
    }

    // simulateCycle() accepts a random engine and distributions.
    void simulateCycle(std::default_random_engine &generator,
                         std::normal_distribution<double> &firmPriceDist,
                         std::uniform_int_distribution<int> &goodsQuantityDist,
                         std::normal_distribution<double> &consumerPriceDist)
    {
        std::cout << "=== Day " << currentCycle + 1 << " ===" << std::endl;
        
        // Process FisherMen: act and update.
        for (auto &fisher : fishers) {
            if (fisher->isActive())
                fisher->act();
        }
        for (auto &fisher : fishers) {
            if (fisher->isActive())
                fisher->update();
        }
        fishers.erase(std::remove_if(fishers.begin(), fishers.end(),
            [](const std::shared_ptr<FisherMan>& f){ return !f->isActive(); }),
            fishers.end());
        
        // Process firms: act and update.
        for (auto &firm : firms) {
            if (firm->isActive())
                firm->act();
        }
        for (auto &firm : firms) {
            if (firm->isActive())
                firm->update();
        }
        firms.erase(std::remove_if(firms.begin(), firms.end(),
            [](const std::shared_ptr<Firm>& f){ return !f->isActive(); }),
            firms.end());
        
        // Population management: Birth new FisherMen.
        for (int i = 0; i < 10; i++) {
            double r = static_cast<double>(rand()) / RAND_MAX;
            if (r < birthRate) {
                int newID = 1000 + fishers.size();
                std::shared_ptr<FisherMan> newFisher = std::make_shared<FisherMan>(
                    newID, 1000.0, 80, 0.0, 0.0, 1.0, 1.0, false, 0.0, 0.0, "fishing", 1, 1, 1
                );
                addFisherMan(newFisher);
            }
        }
        
        // --- JOB MARKET PROCESS ---
        // Each firm submits its job posting.
        for (auto &firm : firms) {
            // generateJobPosting() is implemented in FishingFirm (and declared in Firm as pure virtual).
            JobPosting posting = firm->generateJobPosting("fishing", 1, 1, 1);
            jobMarket->submitJobPosting(posting);
        }
        // Only unemployed fishermen submit job applications.
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
        // Only assign jobs to fishermen who are unemployed; do not reset employment status.
        for (auto &fisher : fishers) {
            if (!fisher->isEmployed() && matches > 0) {
                fisher->setEmployed(true);
                fisher->setWage(dailyWage);
                matches--;
            }
        }
        jobMarket->print();
        
        // --- FISHING MARKET PROCESS ---
        for (auto &firm : firms) {
            double newPrice = firmPriceDist(generator);
            firm->setPriceLevel(newPrice);
            firm->setWageExpense(clearingWage);
            // Assume firms are FishingFirm.
            FishOffering offer = dynamic_cast<FishingFirm*>(firm.get())->generateGoodsOffering(2.0);
            fishingMarket->submitFishOffering(offer);
        }
        for (auto &fisher : fishers) {
            FishOrder order;
            order.id = fisher->getID();
            order.desiredSector = "fishing";
            order.quantity = goodsQuantityDist(generator);
            order.perceivedValue = consumerPriceDist(generator);
            fishingMarket->submitFishOrder(order);
        }
        fishingMarket->clearMarket(generator);
        fishingMarket->print();
        
        // --- MACROECONOMIC INDICATORS ---
        double dailyGDP = 0.0;
        for (auto &firm : firms) {
            dailyGDP += firm->getRevenue();
        }
        GDP = dailyGDP;
        
        int unemployedCount = 0;
        for (const auto &fisher : fishers) {
            if (!fisher->isEmployed()) unemployedCount++;
            }
        unemploymentRate = static_cast<double>(unemployedCount) / fishers.size();

        
        static double prevFishPrice = fishingMarket->getClearingFishPrice();
        double currFishPrice = fishingMarket->getClearingFishPrice();
        inflation = (prevFishPrice > 0) ? (currFishPrice - prevFishPrice) / prevFishPrice : 0.0;
        prevFishPrice = currFishPrice;
        
        std::cout << "Day " << currentCycle + 1 << " Summary:" << std::endl;
        std::cout << "  GDP: " << GDP << std::endl;
        std::cout << "  Unemployment Rate: " << unemploymentRate * 100 << "%" << std::endl;
        std::cout << "  Inflation: " << inflation * 100 << "%" << std::endl;
        std::cout << "=====================================" << std::endl;
        
        currentCycle++;
        jobMarket->reset();
        fishingMarket->reset();
    }

    
    // Print overall world state.
    void printWorldState() const {
        std::cout << "=== World State at Day " << currentCycle << " ===" << std::endl;
        std::cout << "FisherMen: " << fishers.size() << std::endl;
        std::cout << "FishingFirms: " << firms.size() << std::endl;
        jobMarket->print();
        fishingMarket->print();
    }
};

#endif // WORLD_H
