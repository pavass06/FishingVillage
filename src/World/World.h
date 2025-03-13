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
    double annualBirthRate;  // Annual birth rate (e.g., 0.02 for 2%)

    std::vector<std::shared_ptr<FisherMan>> fishers;
    // For firms, we will now use a pointer to a vector of FishingFirm pointers.
    const std::vector<std::shared_ptr<FishingFirm>>* firms; 

    std::shared_ptr<JobMarket> jobMarket;
    std::shared_ptr<FishingMarket> fishingMarket;

    double GDP;
    double unemploymentRate;
    double inflation;

    int maxStarvingDays;  // Maximum consecutive days without eating before death

    std::unordered_map<int, int> daysWithoutEat; // Mapping: fisherID -> consecutive days without eating

public:
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
          GDP(0.0),
          unemploymentRate(0.0),
          inflation(0.0),
          maxStarvingDays(maxStarvingDays_),
          firms(nullptr)
    {}

    // Setter: assign the vector of FishingFirm pointers.
    void setFirms(const std::vector<std::shared_ptr<FishingFirm>>& firmVec) {
         firms = &firmVec;
    }

    const std::vector<std::shared_ptr<FisherMan>>& getFishers() const {
        return fishers;
    }

    int getTotalFishers() const {
        return static_cast<int>(fishers.size());
    }

    double getGDP() const {
        return GDP;
    }

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
        daysWithoutEat[f->getID()] = 0;
    }

    // World still adds firms to its local vector of base class pointers if needed.
    // (However, for supply refresh, we use the separate vector passed via setFirms.)
    void addFirm(std::shared_ptr<Firm> f) {
        // We don't use this vector for supply refresh.
        // (Optional: you can store them as well if needed.)
    }
    
    // simulateCycle processes one simulation day.
    void simulateCycle(std::default_random_engine &generator,
                   std::normal_distribution<double> &firmPriceDist,
                   std::uniform_int_distribution<int> &goodsQuantityDist,
                   std::normal_distribution<double> &consumerPriceDist) {
    // Refresh supply: call refreshSupply on fishingMarket using the firms vector.
    if (firms != nullptr) {
        fishingMarket->refreshSupply(*firms);
    } else {
        std::cerr << "Error: Firms vector not set in World." << std::endl;
    }

         // --- Debug print for cycle.
#if verbose==1
         std::cout << "=== Day " << currentCycle + 1 << " ===" << std::endl;
#endif
         // 1) Process FisherMen.
         for (auto &fisher : fishers) {
             if (fisher->isActive())
                 fisher->act(); // For example: credits wages.
         }
         for (auto &fisher : fishers) {
             if (fisher->isActive())
                 fisher->update();
         }
         fishers.erase(std::remove_if(fishers.begin(), fishers.end(),
             [](const std::shared_ptr<FisherMan>& f) {
                 return !f->isActive();
             }), fishers.end());

         // 2) Process Population: Create new fishermen (using Poisson distribution).
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

         // 3) Process Job Market.
         for (auto &firm : *firms) {  // Use the vector from World.
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
         jobMarket->reset();

         // 4) Process Fishing Market Orders.
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

         // 5) Compute daily GDP as the sum of all firm revenues.
         double dailyGDP = 0.0;
         // (Assuming firms' revenue is tracked elsewhere.)
         GDP = dailyGDP; // Here, you would sum the revenues of firms.
         
         // 6) Calculate unemployment rate.
         int unemployedCount = 0;
         for (const auto &fisher : fishers) {
             if (!fisher->isEmployed())
                 unemployedCount++;
         }
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
             if (daysWithoutEat[fID] >= maxStarvingDays) {
                 fisher->setActive(false);
             }
         }
         fishers.erase(std::remove_if(fishers.begin(), fishers.end(),
             [](const std::shared_ptr<FisherMan>& f) {
                 return !f->isActive();
             }), fishers.end());

         // Update cycle counter.
         currentCycle++;
    }

    void printWorldState() const {
        std::cout << "=== World State at Day " << currentCycle << " ===" << std::endl;
        std::cout << "FisherMen: " << getTotalFishers() << std::endl;
        std::cout << "Job Market: " << std::endl;
        jobMarket->print();
        std::cout << "Fishing Market: " << std::endl;
        fishingMarket->print();
    }
};

#endif // WORLD_H
