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
    int currentCycle;        // Current simulation day
    int totalCycles;         // Total simulation days
    double annualBirthRate;  // Annual birth rate (e.g., 0.02)
    int maxStarvingDays;     // Maximum consecutive days without food before death

    // Population vector.
    std::vector<std::shared_ptr<FisherMan>> fishers;
    // Pointer to vector of FishingFirm objects.
    const std::vector<std::shared_ptr<FishingFirm>>* firms;

    std::shared_ptr<JobMarket> jobMarket;
    std::shared_ptr<FishingMarket> fishingMarket;

    // Macro variables.
    double GDP;
    double unemploymentRate;
    double inflation;

    // Inflation calculation variables.
    double currentOfferMean;
    double currentPerceivedMean;
    // Inflation adjustment parameters:
    double meanAugmentationInflat;
    double varianceAugmentationInflat;
    double meanDiminutionInflat;
    double varianceDiminutionInflat;
    // (Optional) A vector to store daily inflation values.
    std::vector<double> inflations;

    // Mapping: fisherID -> consecutive days without eating.
    std::unordered_map<int, int> daysWithoutEat;

public:
    // Constructor now accepts additional parameters for inflation.
    World(int cycles,
          double annualBirthRate_,
          std::shared_ptr<JobMarket> jm,
          std::shared_ptr<FishingMarket> fm,
          int maxStarvingDays_,
          double offeredPriceMean,
          double perceivedPriceMean,
          double meanAugIn,
          double varAugIn,
          double meanDimIn,
          double varDimIn)
        : currentCycle(0),
          totalCycles(cycles),
          annualBirthRate(annualBirthRate_),
          maxStarvingDays(maxStarvingDays_),
          jobMarket(jm),
          fishingMarket(fm),
          GDP(0.0),
          unemploymentRate(0.0),
          inflation(0.0),
          firms(nullptr),
          currentOfferMean(offeredPriceMean),
          currentPerceivedMean(perceivedPriceMean),
          meanAugmentationInflat(meanAugIn),
          varianceAugmentationInflat(varAugIn),
          meanDiminutionInflat(meanDimIn),
          varianceDiminutionInflat(varDimIn)
    {}

    // Setter for firms vector.
    void setFirms(const std::vector<std::shared_ptr<FishingFirm>> &firmVec) {
         firms = &firmVec;
    }

    // Add a fisherman to the world.
    void addFisherMan(std::shared_ptr<FisherMan> f) {
        fishers.push_back(f);
        daysWithoutEat[f->getID()] = 0;
    }

    int getTotalFishers() const {
        return static_cast<int>(fishers.size());
    }

    double getGDP() const { return GDP; }

    double getUnemploymentRate () const { return unemploymentRate; }
    
    double getInflation(int day) const {return inflations[day];}

    int getUnemployedFishers() const {
        int count = 0;
        for (const auto &fisher : fishers)
            if (!fisher->isEmployed())
                count++;
        return count;
    }

    // The core simulation cycle.
    void simulateCycle(std::default_random_engine &generator,
                       std::normal_distribution<double> &firmPriceDist,
                       std::uniform_int_distribution<int> &goodsQuantityDist,
                       std::normal_distribution<double> &consumerPriceDist) {
        // 0) Refresh supply: update each firm's stock and generate new offerings.
        if (firms != nullptr) {
            fishingMarket->refreshSupply(*firms);
        } else {
            std::cerr << "Error: Firms vector not set in World." << std::endl;
        }

        // 1) Process actions for all active fishers.
        for (auto &fisher : fishers) {
            if (fisher->isActive())
                fisher->act();
        }
        for (auto &fisher : fishers) {
            if (fisher->isActive())
                fisher->update();
        }
        fishers.erase(std::remove_if(fishers.begin(), fishers.end(),
            [](const std::shared_ptr<FisherMan> &f) { return !f->isActive(); }),
            fishers.end());

        // 2) Population management: new births using a Poisson process.
        {
            double normalizedAnnualBirthRate = (annualBirthRate > 1.0) ? annualBirthRate / 100.0 : annualBirthRate;
            double dailyBirthRate = normalizedAnnualBirthRate / 365.0;
            int currentPopulation = getTotalFishers();
            double lambda = dailyBirthRate * currentPopulation;
            std::poisson_distribution<int> poissonDist(lambda);
            int newBirths = poissonDist(generator);
            for (int i = 0; i < newBirths; i++) {
                int newID = 1000 + fishers.size();
                auto newFisher = std::make_shared<FisherMan>(
                    newID, 10.0, 365 * 60, 0.0, 0.0, 1.0, 1.0, false,
                    10.0, 0.0, "fishing", 1, 1, 1
                );
                addFisherMan(newFisher);
            }
        }

        // 3) Job Market process.
        if (firms != nullptr) {
            for (auto &firm : *firms) {
                JobPosting posting = firm->generateJobPosting("fishing", 1, 1, 1);
                jobMarket->submitJobPosting(posting);
            }
        }
        for (auto &fisher : fishers) {
            if (!fisher->isEmployed()) {
                JobApplication app = fisher->generateJobApplication();
                jobMarket->submitJobApplication(app);
            }
        }
        jobMarket->clearMarket(generator);
        double clearingWage = jobMarket->getClearingWage();
        double dailyWage = 1.0 * clearingWage;
        int matches = jobMarket->getMatchedJobs();
        for (auto &fisher : fishers) {
            if (!fisher->isEmployed() && matches > 0) {
                fisher->setEmployed(true);
                fisher->setWage(dailyWage);
                matches--;
            }
        }
        jobMarket->reset();

        // 4) Fishing Market orders: each fisher orders 1 fish.
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
        fishingMarket->setAggregateDemand(static_cast<double>(getTotalFishers()));
        fishingMarket->clearMarket(generator);
        fishingMarket->reset();

        // 5) Compute daily GDP: sum revenue from each firm.
        double dailyGDP = 0.0;
        if (firms != nullptr) {
            for (auto &firm : *firms) {
                dailyGDP += firm->getRevenue();
                firm->resetSales();
            }
        }
        GDP = dailyGDP;

        // 6) Compute unemployment rate.
        int unemployedCount = 0;
        for (const auto &fisher : fishers)
            if (!fisher->isEmployed())
                unemployedCount++;
        unemploymentRate = (fishers.size() > 0) ? static_cast<double>(unemployedCount) / fishers.size() : 0.0;

        // 7) Starvation process.
        std::unordered_map<int, double> purchases = fishingMarket->getPurchases();
        for (auto &fisher : fishers) {
            int fID = fisher->getID();
            if (purchases.find(fID) == purchases.end() || purchases[fID] < 1.0)
                daysWithoutEat[fID]++;
            else
                daysWithoutEat[fID] = 0;
        }
        for (auto &fisher : fishers) {
            int fID = fisher->getID();
            if (daysWithoutEat[fID] >= maxStarvingDays)
                fisher->setActive(false);
        }
        fishers.erase(std::remove_if(fishers.begin(), fishers.end(),
            [](const std::shared_ptr<FisherMan>& f) { return !f->isActive(); }),
            fishers.end());

        // 8) Inflation calculation.
        // Compute the market's aggregate supply and demand to derive the adjustment factor.
        double aggSupply = fishingMarket->getAggregateSupply();
        double aggDemand = fishingMarket->getAggregateDemand();
        double ratio = (aggSupply > 0) ? aggDemand / aggSupply : 1.0;
        double factor = 1.0;
        if (ratio == 1) {
            factor = 1;
        } else if (ratio > 1.0) {
            std::normal_distribution<double> adjustDist(meanAugmentationInflat, varianceAugmentationInflat);
            factor = adjustDist(generator);
        } else if (ratio < 1.0) {
            std::normal_distribution<double> adjustDist(meanDiminutionInflat, varianceDiminutionInflat);
            factor = adjustDist(generator);
        }

        double oldOfferMean = currentOfferMean;
        currentOfferMean *= factor;
        currentPerceivedMean *= factor;
        // Assuming firmPriceDist and consumerPriceDist are updated externally,
        // their parameters could be updated here if needed.
        double inflRate = (oldOfferMean > 0) ? (currentOfferMean - oldOfferMean) / oldOfferMean : 0.0;
        inflation = inflRate;
        inflations.push_back(inflRate);

        currentCycle++;
    }

    void runSimulation(std::default_random_engine &generator,
                       std::normal_distribution<double> &firmPriceDist,
                       std::uniform_int_distribution<int> &goodsQuantityDist,
                       std::normal_distribution<double> &consumerPriceDist) {
        for (int day = 0; day < totalCycles; day++) {
            simulateCycle(generator, firmPriceDist, goodsQuantityDist, consumerPriceDist);
        }
    }

    void printWorldState() const {
        std::cout << "=== World State at Day " << currentCycle << " ===" << std::endl;
        std::cout << "Population: " << getTotalFishers() << std::endl;
        std::cout << "GDP: " << GDP << std::endl;
        std::cout << "Unemployment Rate: " << unemploymentRate * 100 << "%" << std::endl;
        std::cout << "Inflation: " << inflation * 100 << "%" << std::endl;
        std::cout << "Job Market:" << std::endl;
        jobMarket->print();
        std::cout << "Fishing Market:" << std::endl;
        fishingMarket->print();
    }
};

#endif // WORLD_H
