#include <iostream>
#include <vector>
#include <memory>
#include <random>
#include <ctime>
#include "World.h"
#include "FishingFirm.h"
#include "FisherMan.h"
#include "JobMarket.h"
#include "FishingMarket.h"

using namespace std;

// Simulation parameters structure
struct SimulationParameters {
    int totalCycles = 100;          // Total simulation cycles (days)
    int totalFisherMen = 10000;     // Total number of fishermen
    int totalFirms = 500;           // Total number of fishing firms
    int initialEmployed = 90;       // Number of employed fishermen at start
    double initialWage = 5.0;       // Initial base wage for employed fishermen
};

// Simulation class encapsulating the simulation logic
class Simulation {
private:
    SimulationParameters params;
    shared_ptr<JobMarket> jobMarket;
    shared_ptr<FishingMarket> fishingMarket;
    World world;
    vector<shared_ptr<FishingFirm>> firms;
    vector<shared_ptr<FisherMan>> fishers;

    // Random number generator
    default_random_engine generator;

    // Normal distributions (mean, standard deviation):
    // Firm funds follow N(100, 20)
    std::normal_distribution<double> firmFundsDist;
    // Firm stock follows N(50, 10)
    std::normal_distribution<double> firmStockDist;
    // Firm price follows N(6, 0.5)
    std::normal_distribution<double> firmPriceDist;

    // Fisherman lifetime follows N(60, 5)
    std::normal_distribution<double> fisherLifetimeDist;
    // Fisherman age follows N(30, 10)
    std::normal_distribution<double> fisherAgeDist;

    // Consumer price follows N(5, 0.8)
    std::normal_distribution<double> consumerPriceDist;
    // Goods quantity follows a uniform distribution between 1 and 3
    std::uniform_int_distribution<int> goodsQuantityDist;

public:
    // Constructor: initialize simulation parameters and distributions
    Simulation(const SimulationParameters &p)
        : params(p),
          jobMarket(make_shared<JobMarket>(p.initialWage)),
          fishingMarket(make_shared<FishingMarket>(6.0)),
          world(p.totalCycles, 0.1, jobMarket, fishingMarket),
          generator(static_cast<unsigned int>(time(0))),
          firmFundsDist(100.0, 20.0),
          firmStockDist(50.0, 10.0),
          firmPriceDist(6.0, 0.5),
          fisherLifetimeDist(60.0, 5.0),
          fisherAgeDist(30.0, 10.0),
          consumerPriceDist(5.0, 0.8),
          goodsQuantityDist(1, 3)
    {
        // Initialize FishingFirms
        for (int id = 100; id < 100 + params.totalFirms; id++) {
            double funds = firmFundsDist(generator);   // Draw funds from N(100, 20)
            double stock = firmStockDist(generator);     // Draw stock from N(50, 10)
            int lifetime = 365;                          // Fixed firm lifetime (days)
            double salesEff = 2.0;                       // Sales efficiency factor
            double jobMult = 0.05;                       // Job multiplier for vacancies
            double price = firmPriceDist(generator);     // Draw price from N(6, 0.5)
            
            auto firm = make_shared<FishingFirm>(id, funds, lifetime, 18, stock, salesEff, jobMult);
            firm->setPriceLevel(price);
            firms.push_back(firm);
            world.addFirm(firm);
        }

        // Initialize employed FisherMen
        for (int id = 0; id < params.initialEmployed; id++) {
            double age = fisherAgeDist(generator);           // Draw age from N(30, 10)
            double lifetime = fisherLifetimeDist(generator);   // Draw lifetime from N(60, 5)
            
            auto fisher = make_shared<FisherMan>(
                id,               
                0.0,              // Initial funds
                lifetime,         
                0.0,              // Income (unused)
                0.0,              // Savings
                1.0,              // Job demand
                1.0,              // Goods demand
                true,             // Employed
                params.initialWage,
                0.0,              // Unemployment benefit
                "fishing",        // Job sector
                1,                // Education level
                1,                // Experience level
                1                 // Job preference
            );
            fishers.push_back(fisher);
            world.addFisherMan(fisher);
        }
        
        // Initialize unemployed FisherMen
        for (int id = params.initialEmployed; id < params.totalFisherMen; id++) {
            double age = fisherAgeDist(generator);
            double lifetime = fisherLifetimeDist(generator);
            
            auto fisher = make_shared<FisherMan>(
                id,               
                0.0,
                lifetime,
                0.0,
                0.0,
                1.0,
                1.0,
                false,            // Not employed
                0.0,
                0.0,
                "fishing",
                1,
                1,
                1
            );
            fishers.push_back(fisher);
            world.addFisherMan(fisher);
        }
    }

    // Run the simulation cycles
    void run() {
        vector<double> GDPs;
        vector<double> unemploymentRates;
        vector<double> inflations;
        
        for (int day = 0; day < params.totalCycles; day++) {
            cout << "===== Day " << day + 1 << " =====" << endl;
            // Run one simulation cycle using the stored distributions for randomness
            world.simulateCycle(generator, firmPriceDist, goodsQuantityDist, consumerPriceDist);
            
            // Retrieve GDP from the world object
            double dailyGDP = world.getGDP();
            GDPs.push_back(dailyGDP);
            
            // Calculate unemployment rate
            int totalFishers = world.getTotalFishers();
            int unemployedFishers = world.getUnemployedFishers();
            double dailyUnemploymentRate = (totalFishers > 0) ? 
                (static_cast<double>(unemployedFishers) / totalFishers) * 100.0 : 0.0;
            unemploymentRates.push_back(dailyUnemploymentRate);
            
            // Compute inflation rate using the fish market clearing price
            static double prevFishPrice = fishingMarket->getClearingFishPrice();
            double currFishPrice = fishingMarket->getClearingFishPrice();
            double inflRate = (prevFishPrice > 0) ? 
                (currFishPrice - prevFishPrice) / prevFishPrice : 0.0;
            inflations.push_back(inflRate);
            prevFishPrice = currFishPrice;
        }
        
        // Output the collected data
        cout << "GDP Values = [";
        for (size_t i = 0; i < GDPs.size(); i++) {
            cout << GDPs[i];
            if (i != GDPs.size() - 1) cout << ", ";
        }
        cout << "]\n";
        
        cout << "Unemployment Rates = [";
        for (size_t i = 0; i < unemploymentRates.size(); i++) {
            cout << unemploymentRates[i];
            if (i != unemploymentRates.size() - 1) cout << ", ";
        }
        cout << "]\n";
        
        cout << "Inflation Rates = [";
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
