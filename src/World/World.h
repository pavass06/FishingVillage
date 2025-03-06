#ifndef WORLD_H
#define WORLD_H

#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>
#include <cstdlib>
#include <random>
#include <unordered_map>  // For tracking daysWithoutEat
#include "FisherMan.h"
#include "Firm.h"
#include "FishingFirm.h"
#include "JobMarket.h"
#include "FishingMarket.h"

class World {
private:
    int currentCycle;        // Current simulation day
    int totalCycles;         // Total simulation days
    double annualBirthRate;  // Annual birth rate (e.g., 0.02 for 2%)

    std::vector<std::shared_ptr<FisherMan>> fishers;
    std::vector<std::shared_ptr<Firm>> firms;
    
    std::shared_ptr<JobMarket> jobMarket;
    std::shared_ptr<FishingMarket> fishingMarket;

    double previousFishPrice;  // For inflation calculation
    double GDP;
    double unemploymentRate;
    double inflation;

    int maxStarvingDays;  // Maximum consecutive days without eating before death
    // Mapping: fisherID -> consecutive days without eating
    std::unordered_map<int, int> daysWithoutEat;

public:
    // Constructor now accepts maxStarvingDays as a parameter.
    World(int cycles,
          double annualBirthRate_,
          std::shared_ptr<JobMarket> jm,
          std::shared_ptr<FishingMarket> fm,
          int maxStarvingDays_)
        : currentCycle(0),
          totalCycles(cycles),
          annualBirthRate(annualBirthRate_),
          jobMarket(jm),
          fishingMarket(fm),
          previousFishPrice(fm->getClearingFishPrice()),
          GDP(0.0),
          unemploymentRate(0.0),
          inflation(0.0),
          maxStarvingDays(maxStarvingDays_)
    {}

    const std::vector<std::shared_ptr<FisherMan>>& getFishers() const {
        return fishers;
    }

    int getTotalFishers() const {
        return fishers.size();
    }

    double getGDP() const {
        return GDP;
    }

    int getUnemployedFishers() const {
        int count = 0;
        for (const auto &fisher : fishers) {
            if (!fisher->isEmployed()) {
                count++;
            }
        }
        return count;
    }

    // Adds a fisherman and initializes his starvation counter.
    void addFisherMan(std::shared_ptr<FisherMan> f) {
        fishers.push_back(f);
        daysWithoutEat[f->getID()] = 0;
    }

    void addFirm(std::shared_ptr<Firm> f) {
        firms.push_back(f);
    }



    // simulateCycle() processes one simulation day.
    void simulateCycle(std::default_random_engine &generator,
                       std::normal_distribution<double> &firmPriceDist,
                       std::uniform_int_distribution<int> &goodsQuantityDist,
                       std::normal_distribution<double> &consumerPriceDist)
    {
#if verbose==1
        std::cout << "=== Day " << currentCycle + 1 << " ===" << std::endl;
#endif
        // 1) Process FisherMen: Call act() to credit wages, then update() for consumption and aging.
        for (auto &fisher : fishers) {
            if (fisher->isActive())
                fisher->act();   // Adds wage to funds
        }
        for (auto &fisher : fishers) {
            if (fisher->isActive())
                fisher->update();
        }
        // Remove fishermen who have become inactive (e.g., died or aged out).
        fishers.erase(std::remove_if(fishers.begin(), fishers.end(),
            [](const std::shared_ptr<FisherMan> &f) {
                return !f->isActive();
            }),
            fishers.end());
        
        // 2) Process Firms: Call act() and update(), then remove inactive ones.
        for (auto &firm : firms) {
            if (firm->isActive())
                firm->act();
        }
        for (auto &firm : firms) {
            if (firm->isActive())
                firm->update();
        }
        firms.erase(std::remove_if(firms.begin(), firms.end(),
            [](const std::shared_ptr<Firm> &f) {
                return !f->isActive();
            }),
            firms.end());
        
        // 3) Population management: Create new fishermen using a Poisson distribution.
        {
            double dailyBirthRate = annualBirthRate / 365.0;
            int currentPopulation = static_cast<int>(fishers.size());
            double lambda = dailyBirthRate * currentPopulation;
            std::poisson_distribution<int> poissonDist(lambda);
            int newBirths = poissonDist(generator);
            for (int i = 0; i < newBirths; i++) {
                int newID = 1000 + fishers.size();
                auto newFisher = std::make_shared<FisherMan>(
                    newID,         // ID
                    0.0,           // Initial funds
                    365 * 60,      // Lifespan in days (e.g., 60 years)
                    0.0,           // Income (unused)
                    0.0,           // Savings
                    1.0,           // Job demand
                    1.0,           // Goods demand
                    false,         // Initially unemployed
                    0.0,           // Wage (will be set later)
                    0.0,           // Unemployment benefit (omitted)
                    "fishing",     // Job sector
                    1, 1, 1        // Education, Experience, Job preference
                );
                addFisherMan(newFisher);
            }
        }

        // 4) Job market process: Firms post jobs; unemployed fishermen submit applications.
        for (auto &firm : firms) {
            JobPosting posting = firm->generateJobPosting("fishing", 1, 1, 1);
            jobMarket->submitJobPosting(posting);
        }
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
        jobMarket->print();
        jobMarket->reset();
        
        // 5) Fishing market process: Firms submit fish offerings and fishermen submit orders.
        for (auto &firm : firms) {
            double newPrice = firmPriceDist(generator);
            firm->setPriceLevel(newPrice);
            firm->setWageExpense(clearingWage);
            // Generate an offering; parameter (e.g., 2.0) can be adjusted.
            FishOffering offer = dynamic_cast<FishingFirm*>(firm.get())->generateGoodsOffering(2.0);
            offer.firm = std::dynamic_pointer_cast<FishingFirm>(firm);
            fishingMarket->submitFishOffering(offer);
        }
        for (auto &fisher : fishers) {
            FishOrder order;
            order.id = fisher->getID();
            order.desiredSector = "fishing";
            order.quantity = 1 ; 
            order.perceivedValue = consumerPriceDist(generator);
            order.availableFunds = fisher->getFunds();
            // Set hungry to true if the fisher's daysWithoutEat counter is not 0.
            order.hungry = (daysWithoutEat[fisher->getID()] > 0);
            fishingMarket->submitFishOrder(order);
        }

        fishingMarket->clearMarket(generator);
        fishingMarket->print();
        fishingMarket->reset();
        
        // 6) Compute daily GDP as the sum of firm revenues, then reset each firm's sales.
        double dailyGDP = 0.0;
        for (auto &firm : firms) {
            dailyGDP += firm->getRevenue();
        }
        GDP = dailyGDP;
        for (auto &firm : firms) {
            firm->resetSales();
        }
        
        // 7) Calculate the unemployment rate.
        int unemployedCount = 0;
        for (const auto &fisher : fishers) {
            if (!fisher->isEmployed())
                unemployedCount++;
        }
        unemploymentRate = (fishers.size() > 0)
                           ? static_cast<double>(unemployedCount) / fishers.size()
                           : 0.0;

        // 8) Calculate inflation based on changes in the fish market's clearing price.
        static double prevFishPrice = fishingMarket->getClearingFishPrice();
        double currFishPrice = fishingMarket->getClearingFishPrice();
        inflation = (prevFishPrice > 0.0)
                    ? (currFishPrice - prevFishPrice) / prevFishPrice
                    : 0.0;
        prevFishPrice = currFishPrice;

        // 9) Starvation Check:
        // Retrieve the mapping of purchases (fisherID -> total fish bought) for this cycle.
        std::unordered_map<int, double> purchases = fishingMarket->getPurchases();

        // Update the starvation counter for each fisherman.
        for (auto &fisher : fishers) {
            int fID = fisher->getID();
            if (purchases.find(fID) == purchases.end() || purchases[fID] < 1.0) {
                // If the fisherman did not purchase at least 1 fish, increment his starvation counter.
                daysWithoutEat[fID]++;
            } else {
                // Otherwise, reset the counter.
                daysWithoutEat[fID] = 0;
            }
        }

        // Mark fishermen as inactive if they exceed the maximum allowed days without eating.
        for (auto &fisher : fishers) {
            int fID = fisher->getID();
            if (daysWithoutEat[fID] >= maxStarvingDays) {
                fisher->setActive(false);
            }
        }
        // Remove inactive fishermen.
        fishers.erase(std::remove_if(fishers.begin(), fishers.end(),
            [](const std::shared_ptr<FisherMan>& f) {
                return !f->isActive();
            }),
            fishers.end());

        // Print the macro summary for the day.
#if verbose==1
        std::cout << "-----------" << std::endl;
        std::cout << "Day " << currentCycle + 1 << " Summary:" << std::endl;
        std::cout << "Population: " << getTotalFishers() << std::endl;
        std::cout << "  GDP: " << GDP << std::endl;
        std::cout << "  Unemployment Rate: " << unemploymentRate * 100 << "%" << std::endl;
        std::cout << "  Inflation: " << inflation * 100 << "%" << std::endl;
        std::cout << "=====================================" << std::endl;
#endif 
        currentCycle++;
        // Reset the markets for the next cycle.
        jobMarket->reset();
        fishingMarket->reset();
    }

    void printWorldState() const {
        std::cout << "=== World State at Day " << currentCycle << " ===" << std::endl;
        std::cout << "FisherMen: " << fishers.size() << std::endl;
        std::cout << "FishingFirms: " << firms.size() << std::endl;
        jobMarket->print();
        fishingMarket->print();
    }
};

#endif // WORLD_H
