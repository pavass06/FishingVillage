#ifndef WORLD_H
#define WORLD_H

#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>
#include <cstdlib>
#include <random>            // Needed for Poisson distribution
#include "FisherMan.h"
#include "Firm.h"
#include "FishingFirm.h"
#include "JobMarket.h"
#include "FishingMarket.h"

class World {
private:
    int currentCycle;         // Each cycle represents 1 day.
    int totalCycles;          // Total number of days.

    // We’ll treat this as an ANNUAL birth rate (2% = 0.02).
    // The daily birth rate will be annualBirthRate / 365 in the actual cycle.
    double annualBirthRate;

    std::vector<std::shared_ptr<FisherMan>> fishers;
    std::vector<std::shared_ptr<Firm>> firms;
    
    std::shared_ptr<JobMarket> jobMarket;
    std::shared_ptr<FishingMarket> fishingMarket;

    double previousFishPrice; // For inflation calculations

    double GDP;
    double unemploymentRate;  // Total unemployed / total fishermen
    double inflation;         // Percentage change in fish market clearing price

public:
    // NOTE: We changed the constructor to accept an ANNUAL birth rate (e.g., 0.02).
    World(int cycles, double annualBirthRate_,
          std::shared_ptr<JobMarket> jm, std::shared_ptr<FishingMarket> fm)
        : currentCycle(0),
          totalCycles(cycles),
          annualBirthRate(annualBirthRate_),
          jobMarket(jm),
          fishingMarket(fm),
          previousFishPrice(fm->getClearingFishPrice()),
          GDP(0.0),
          unemploymentRate(0.0),
          inflation(0.0)
    {}

    const std::vector<std::shared_ptr<FisherMan>>& getFishers() const {
        return fishers;
    }

    // Returns the total number of fishermen in the world.
    int getTotalFishers() const {
        return fishers.size();
    }

    // Returns the total GDP.
    double getGDP() const {
        return GDP;
    }

    // Returns the number of unemployed fishermen.
    int getUnemployedFishers() const {
        int count = 0;
        for (const auto &fisher : fishers) {
            if (!fisher->isEmployed()) {
                count++;
            }
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
        
        // 1) Process FisherMen: act and update
        for (auto &fisher : fishers) {
            if (fisher->isActive()) {
                fisher->act();
            }
        }
        for (auto &fisher : fishers) {
            if (fisher->isActive()) {
                fisher->update();
            }
        }
        // Remove inactive fishermen
        fishers.erase(std::remove_if(fishers.begin(), fishers.end(),
            [](const std::shared_ptr<FisherMan> &f){
                return !f->isActive();
            }),
            fishers.end());
        
        // 2) Process Firms: act and update
        for (auto &firm : firms) {
            if (firm->isActive()) {
                firm->act();
            }
        }
        for (auto &firm : firms) {
            if (firm->isActive()) {
                firm->update();
            }
        }
        // Remove inactive firms
        firms.erase(std::remove_if(firms.begin(), firms.end(),
            [](const std::shared_ptr<Firm> &f){
                return !f->isActive();
            }),
            firms.end());
        
        // 3) Population management: Poisson-based daily births
        {
            // Convert 2% annual to daily (0.02 / 365 ~= 0.0000548)
            double dailyBirthRate = annualBirthRate / 365.0;
            // Lambda = dailyBirthRate * currentPopulation
            int currentPopulation = static_cast<int>(fishers.size());
            double lambda = dailyBirthRate * currentPopulation;

            // Draw from Poisson(lambda)
            std::poisson_distribution<int> poissonDist(lambda);
            int newBirths = poissonDist(generator);

            for (int i = 0; i < newBirths; i++) {
                int newID = 1000 + fishers.size();
                // For demonstration: these new fishers start with minimal or zero age/funds
                std::shared_ptr<FisherMan> newFisher = std::make_shared<FisherMan>(
                    newID,         // ID
                    0,         // Initial funds (arbitrary)
                    365 * 60,      // Lifespan in days, e.g. 60 years (example)
                    0.0,           // Income
                    0.0,           // Savings
                    1.0,           // Job demand
                    1.0,           // Goods demand
                    false,         // Employed
                    0.0,           // Wage
                    0.0,           // Unemployment benefit
                    "fishing",     // Sector
                    1, 1, 1        // Education, Experience, Preference
                );
                addFisherMan(newFisher);
            }
        }

        // 4) Job market process
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
        
        // 5) Fishing market process
        for (auto &firm : firms) {
            double newPrice = firmPriceDist(generator);
            firm->setPriceLevel(newPrice);
            firm->setWageExpense(clearingWage);
            // Assume these are FishingFirms
            FishOffering offer = dynamic_cast<FishingFirm*>(firm.get())->generateGoodsOffering(2.0);
            offer.firm = std::dynamic_pointer_cast<FishingFirm>(firm);
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
        
        // 6) Compute daily GDP (sum of firm revenues)
        double dailyGDP = 0.0;
        for (auto &firm : firms) {
            dailyGDP += firm->getRevenue();
        }
        GDP = dailyGDP;

        // Reset sales for next cycle
        for (auto &firm : firms) {
            firm->resetSales();
        }
        
        // 7) Unemployment Rate
        int unemployedCount = 0;
        for (const auto &fisher : fishers) {
            if (!fisher->isEmployed()) {
                unemployedCount++;
            }
        }
        unemploymentRate = (fishers.size() > 0)
                           ? static_cast<double>(unemployedCount) / fishers.size()
                           : 0.0;

        // 8) Inflation
        static double prevFishPrice = fishingMarket->getClearingFishPrice();
        double currFishPrice = fishingMarket->getClearingFishPrice();
        inflation = (prevFishPrice > 0.0)
                    ? (currFishPrice - prevFishPrice) / prevFishPrice
                    : 0.0;
        prevFishPrice = currFishPrice;

        // Print the day's macro summary
        std::cout << "-----------" << std::endl;
        std::cout << "Day " << currentCycle + 1 << " Summary:" << std::endl;
        std::cout << "Population: " << getTotalFishers() << std::endl;
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
