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

// Structure to hold simulation parameters.
struct SimulationParameters {
    int totalCycles = 10;          // 10 days
    int totalFisherMen = 100;
    int totalFirms = 5;
    int initialEmployed = 90;       // 90 employed, 10 unemployed
    double initialWage = 5.0;       // Initial base wage
};

// The Simulation class encapsulates the entire simulation logic.
class Simulation {
private:
    SimulationParameters params;
    shared_ptr<JobMarket> jobMarket;
    shared_ptr<FishingMarket> fishingMarket;
    World world;
    vector<shared_ptr<FishingFirm>> firms;
    vector<shared_ptr<FisherMan>> fishers;
    
    // Random engine and distributions.
    default_random_engine generator;
    normal_distribution<double> firmPriceDist;
    normal_distribution<double> consumerPriceDist;
    uniform_int_distribution<int> goodsQuantityDist;
    
public:
    Simulation(const SimulationParameters &p)
    : params(p),
      jobMarket(make_shared<JobMarket>(p.initialWage)),
      fishingMarket(make_shared<FishingMarket>(6.0)),
      world(p.totalCycles, 0.1, jobMarket, fishingMarket),
      generator(static_cast<unsigned int>(time(0))),
      firmPriceDist(6.0, 0.5),
      consumerPriceDist(5.0, 0.8),
      goodsQuantityDist(1, 3)
    {
        // Initialize FishingFirms.
        for (int i = 0; i < params.totalFirms; i++) {
            int employees = 18; // Each firm employs 18 workers.
            double stock = 50.0;
            double price = firmPriceDist(generator);
            // Create a FishingFirm. (Funds: 5000, lifetime: 365, sales efficiency: 2.0, job post multiplier: 0.05)
            auto firm = make_shared<FishingFirm>(300 + i, 5000.0, 365, employees, stock, 2.0, 0.05);
            firm->setPriceLevel(price);
            firms.push_back(firm);
            world.addFirm(firm);
        }
        // Initialize FisherMen.
        // First 90 will be employed.
        for (int i = 0; i < params.initialEmployed; i++) {
            auto fisher = make_shared<FisherMan>(i, 1000.0, 80, 0.0, 0.0, 1.0, 1.0,
                                                  true,  // Employed
                                                  params.initialWage,   // Initial wage
                                                  0.0, "fishing", 1, 1, 1);
            fishers.push_back(fisher);
            world.addFisherMan(fisher);
        }
        // Next 10 will be unemployed.
        for (int i = params.initialEmployed; i < params.totalFisherMen; i++) {
            auto fisher = make_shared<FisherMan>(i, 1000.0, 80, 0.0, 0.0, 1.0, 1.0,
                                                  false, // Unemployed
                                                  0.0,   // No wage
                                                  0.0, "fishing", 1, 1, 1);
            fishers.push_back(fisher);
            world.addFisherMan(fisher);
        }
    }
    
    void run() {
        int totalCycles = params.totalCycles;
        double GDPs[totalCycles] = {0};             // All elements initialized to 0.0
        double unemploymentRates[totalCycles] = {0};
        double inflations[totalCycles] = {0};
        
        // Run the simulation loop.
        for (int day = 0; day < params.totalCycles; day++) {
            cout << "===== Day " << day + 1 << " =====" << endl;
            world.simulateCycle(generator, firmPriceDist, goodsQuantityDist, consumerPriceDist);
            
            // Compute GDP as the sum of revenues from all firms.
            double dailyGDP = 0.0;
            for (auto &firm : firms) {
                dailyGDP += firm->getRevenue();
                }
            GDPs[day] = dailyGDP;
            
            // Compute unemployment rate from the world state:
            int totalFishers = world.getTotalFishers();
            int unemployedCount = world.getUnemployedFishers();
            unemploymentRates[day] = (totalFishers > 0) ? (static_cast<double>(unemployedCount) / totalFishers) * 100.0 : 0.0;
            
            // Compute inflation as the percentage change in fish market clearing price.
            // We use a static variable to keep track of the previous day's price.
            static double prevFishPrice = fishingMarket->getClearingFishPrice();
            double currFishPrice = fishingMarket->getClearingFishPrice();
            inflations[day] = (prevFishPrice > 0) ? ((currFishPrice - prevFishPrice) / prevFishPrice) * 100.0 : 0.0;
            prevFishPrice = currFishPrice;
          }
        
        // Print the collected data.
        cout << "GDP Values = [";
        for (size_t i = 0; i < GDPs.size(); i++) {
            cout << GDPs[i];
            if (i != GDPs.size() - 1) cout << ", ";
        }
        cout << "]\n";
        
        cout << "Unemployment Values = [";
        for (size_t i = 0; i < unemploymentRates.size(); i++) {
            cout << unemploymentRates[i];
            if (i != unemploymentRates.size() - 1) cout << ", ";
        }
        cout << "]\n";
        
        cout << "Inflation Values = [";
        for (size_t i = 0; i < inflations.size(); i++) {
            cout << inflations[i];
            if (i != inflations.size() - 1) cout << ", ";
        }
        cout << "]\n";
    }
};

int main() {
    SimulationParameters params;
    Simulation sim(params);
    sim.run();
    return 0;
}
