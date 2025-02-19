#include <iostream>
#include <vector>
#include <memory>
#include <random>
#include <ctime>
#include <fstream>
#include <cstdlib>
#include "World.h"
#include "FishingFirm.h"
#include "FisherMan.h"
#include "JobMarket.h"
#include "FishingMarket.h"

using namespace std;

int main() {
    // Declare random engine and distributions.
    default_random_engine generator(static_cast<unsigned int>(time(0)));
    normal_distribution<double> firmPriceDist(6.0, 0.5);
    normal_distribution<double> consumerPriceDist(5.0, 0.8);
    uniform_int_distribution<int> goodsQuantityDist(1, 3);

    // Simulation parameters.
    const int totalCycles = 10;         // 10 days.
    const int totalFisherMen = 100;
    const int totalFirms = 5;
    const int initialEmployed = 90;        // 90 employed, 10 unemployed.
    const double initialWage = 5.0;        // Initial base wage.

    // Create JobMarket and FishingMarket.
    shared_ptr<JobMarket> jobMarket = make_shared<JobMarket>(initialWage);
    shared_ptr<FishingMarket> fishingMarket = make_shared<FishingMarket>(6.0);

    // Create World with a birth rate of 0.1.
    World world(totalCycles, 0.1, jobMarket, fishingMarket);

    // Create 5 FishingFirms, each with 18 employees and stock = 50.
    vector<shared_ptr<FishingFirm>> firms;
    for (int i = 0; i < totalFirms; i++) {
        int employees = 18;
        double stock = 50.0;
        double price = firmPriceDist(generator);
        shared_ptr<FishingFirm> firm = make_shared<FishingFirm>(
            300 + i, 5000.0, 365, employees, stock, 2.0, 0.05
        );
        firm->setPriceLevel(price);
        firms.push_back(firm);
        world.addFirm(firm);
    }

    // Create 100 FisherMen: first 90 employed, remaining 10 unemployed.
 for (int i = 0; i < 90; ++i) {
    auto fisherman = std::make_shared<FisherMan>(i, 1000.0, 80, 0.0, 0.0, 1.0, 1.0,
                                                 true,  // Set employed
                                                 5.0,   // Initial wage
                                                 0.0, "fishing", 1, 1, 1);
    world.addFisherMan(fisherman);
}
for (int i = 90; i < 100; ++i) {
    auto fisherman = std::make_shared<FisherMan>(i, 1000.0, 80, 0.0, 0.0, 1.0, 1.0,
                                                 false, // Unemployed
                                                 0.0,   // No wage
                                                 0.0, "fishing", 1, 1, 1);
    world.addFisherMan(fisherman);
}

    vector<double> GDPs;
    vector<double> unemploymentRates;
    vector<double> inflations;

    // Simulation loop.
    for (int day = 0; day < totalCycles; day++) {
        cout << "===== Day " << day + 1 << " =====" << endl;
        world.simulateCycle(generator, firmPriceDist, goodsQuantityDist, consumerPriceDist);

        double dailyGDP = 0.0;
        for (auto &firm : firms) {
            dailyGDP += firm->getRevenue();
        }
        GDPs.push_back(dailyGDP);
        double totalJobApps = jobMarket->getAggregateDemand();
        int matchedJobs = jobMarket->getMatchedJobs();
        int total = world.getTotalFishers();
        int unemployed = world.getUnemployedFishers();
        double dailyUnemploymentRate = (total > 0) ? (static_cast<double>(unemployed) / total) * 100.0 : 0.0;
        unemploymentRates.push_back(dailyUnemploymentRate);
        static double prevFishPrice = fishingMarket->getClearingFishPrice();
        double currFishPrice = fishingMarket->getClearingFishPrice();
        double inflRate = (prevFishPrice > 0) ? (currFishPrice - prevFishPrice) / prevFishPrice : 0.0;
        inflations.push_back(inflRate);
        prevFishPrice = currFishPrice;
    
    }

   std::cout << "GDP Values = " << "[";
for (size_t i = 0; i < GDPs.size(); i++) {
    std::cout << GDPs[i];
    if (i != GDPs.size() - 1) std::cout << ", ";
}
std::cout << "]\n";

std::cout << "Unemployment Values = " << "[";
for (size_t i = 0; i < unemploymentRates.size(); i++) {
    std::cout << unemploymentRates[i];
    if (i != unemploymentRates.size() - 1) std::cout << ", ";
}
std::cout << "]\n";

std::cout << "Inflation Values = " << "[";
for (size_t i = 0; i < inflations.size(); i++) {
    std::cout << inflations[i];
    if (i != inflations.size() - 1) std::cout << ", ";
}
std::cout << "]\n";
}
