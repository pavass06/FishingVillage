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
    int totalCycles = 5;          // Total simulation cycles (days)
    int totalFisherMen = 100;     // Total number of fishermen
    int totalFirms = 5;           // Total number of fishing firms
    int initialEmployed = 90;     // Number of employed fishermen at start
    double initialWage = 5.0;     // Base wage (here used as fish price input)
};

// Simulation class encapsulating the simulation logic
class Simulation {
private:
    SimulationParameters params;
    // Instantiate JobMarket with: initial wage (unused), fish price, and mean fish order.
    shared_ptr<JobMarket> jobMarket;
    shared_ptr<FishingMarket> fishingMarket;
    World world;
    vector<shared_ptr<FishingFirm>> firms;
    vector<shared_ptr<FisherMan>> fishers;

    // Random number generator
    default_random_engine generator;

    // Normal distributions for firm funds and stock (unchanged)
    std::normal_distribution<double> firmFundsDist; // N(100, 20)
    std::normal_distribution<double> firmStockDist;   // N(50, 10)

    // Evolving means for offered and perceived prices
    double currentOfferMean;     // Evolving mean for firm's offered price (starts at 6.0)
    double currentPerceivedMean; // Evolving mean for consumer's perceived price (starts at 5.0)

    // Distributions for firm offered price and consumer perceived price
    std::normal_distribution<double> firmPriceDist;    // Initially N(6, 0.5)
    std::normal_distribution<double> consumerPriceDist;  // Initially N(5, 0.8)

    // Other distributions remain the same
    std::normal_distribution<double> fisherLifetimeDist; // N(60, 5)
    std::normal_distribution<double> fisherAgeDist;      // N(30, 10)
    std::uniform_int_distribution<int> goodsQuantityDist; // Uniform between 1 and 3

public:
    // Constructor: initialize simulation parameters, markets, and distributions
    Simulation(const SimulationParameters &p)
        : params(p),
          // Note: Pass initial wage, fish price = 5.0, and mean fish order = 1.5.
          jobMarket(make_shared<JobMarket>(p.initialWage, 5.0, 1.5)),
          fishingMarket(make_shared<FishingMarket>(5.0)), // initial fish price = 5.0
          world(p.totalCycles, 0.1, jobMarket, fishingMarket),
          generator(static_cast<unsigned int>(time(0))),
          firmFundsDist(100.0, 20.0),
          firmStockDist(50.0, 10.0),
          // Initialize evolving means with base values
          currentOfferMean(6.0),
          currentPerceivedMean(5.0),
          // Initialize the price distributions with these means
          firmPriceDist(6.0, 0.5),
          consumerPriceDist(5.0, 0.8),
          fisherLifetimeDist(60.0, 5.0),
          fisherAgeDist(30.0, 10.0),
          goodsQuantityDist(1, 3)
    {
        // Initialize FishingFirms
        for (int id = 100; id < 100 + params.totalFirms; id++) {
            double funds = firmFundsDist(generator);   // Draw funds from N(100, 20)
            double stock = firmStockDist(generator);     // Draw stock from N(50, 10)
            int lifetime = 365;                          // Fixed firm lifetime (days)
            double salesEff = 2.0;                       // Sales efficiency factor
            double jobMult = 0.05;                       // Job multiplier for vacancies
            double price = firmPriceDist(generator);     // Draw offered price from N(currentOfferMean, 0.5)
            
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
                params.initialWage, // Initial wage (will be recalculated)
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
        vector<double> gdpPerCapitas;  // GDP per capita for each cycle
        vector<int> populations;       // Population count for each cycle

        // Simulation loop
        for (int day = 0; day < params.totalCycles; day++) {
            cout << "===== Day " << day + 1 << " =====" << endl;
            
            // Run one simulation cycle using the current price distributions.
            world.simulateCycle(generator, firmPriceDist, goodsQuantityDist, consumerPriceDist);
            
            // ---- NEW CODE: Update JobMarket based on the latest fish market price ----
            double updatedFishPrice = fishingMarket->getClearingFishPrice();
            jobMarket->setCurrentFishPrice(updatedFishPrice);
            // Recalculate the clearing wage: (fish price) * (mean fish order)
            jobMarket->clearMarket(generator);
            // Print the updated JobMarket state
            jobMarket->print();
            // -------------------------------------------------------------------------
            
            // Retrieve the current population and print it.
            int totalFishers = world.getTotalFishers();
            populations.push_back(totalFishers);
            cout << "Population: " << totalFishers << endl;
            
            // Retrieve daily GDP from the world object.
            double dailyGDP = world.getGDP();
            GDPs.push_back(dailyGDP);
            
            // Calculate GDP per capita: dailyGDP divided by number of fishermen.
            double perCapita = (totalFishers > 0) ? dailyGDP / totalFishers : 0.0;
            gdpPerCapitas.push_back(perCapita);
        
            // Calculate unemployment rate (in percentage).
            int unemployedFishers = world.getUnemployedFishers();
            double dailyUnemploymentRate = (totalFishers > 0) ?
                (static_cast<double>(unemployedFishers) / totalFishers) * 100.0 : 0.0;
            unemploymentRates.push_back(dailyUnemploymentRate);
        
            // Calculate inflation rate based on fish market clearing price.
            static double prevFishPrice = updatedFishPrice;
            double currFishPrice = fishingMarket->getClearingFishPrice();
            double inflRate = (prevFishPrice > 0) ? (currFishPrice - prevFishPrice) / prevFishPrice : 0.0;
            inflations.push_back(inflRate);
            prevFishPrice = currFishPrice;
            
            // --- Update the underlying price means (using adjustment factor) ---
            double aggSupply = fishingMarket->getAggregateSupply();
            double aggDemand = fishingMarket->getAggregateDemand();
            double ratio = (aggSupply > 0) ? aggDemand / aggSupply : 1.0;
            double factor = 1.0;
            if (ratio > 1.0) {
                // Demand exceeds supply: draw factor from N(1.025, 0.005)
                std::normal_distribution<double> adjustDist(1.025, 0.005);
                factor = adjustDist(generator);
            } else if (ratio < 1.0) {
                // Supply exceeds demand: draw factor from N(0.975, 0.005)
                std::normal_distribution<double> adjustDist(0.975, 0.005);
                factor = adjustDist(generator);
            }
            // Update the evolving means.
            currentOfferMean *= factor;
            currentPerceivedMean *= factor;
            // Update the distributions with the new means (standard deviations remain constant).
            firmPriceDist.param(std::normal_distribution<double>::param_type(currentOfferMean, 0.5));
            consumerPriceDist.param(std::normal_distribution<double>::param_type(currentPerceivedMean, 0.8));
        }
        
        // Output collected data (GDP, Population, etc.)
        cout << "GDP Values = [";
        for (size_t i = 0; i < GDPs.size(); i++) {
            cout << GDPs[i];
            if (i != GDPs.size() - 1) cout << ", ";
        }
        cout << "]\n";

        cout << "Population Values = [";
        for (size_t i = 0; i < populations.size(); i++) {
            cout << populations[i];
            if (i != populations.size() - 1) cout << ", ";
        }
        cout << "]\n";
        
        cout << "GDP Growth Values = [";
        vector<double> gdpGrowth;
        for (size_t i = 0; i < GDPs.size() - 1; i++) {
            if (GDPs[i] != 0)
                gdpGrowth.push_back((GDPs[i+1] - GDPs[i]) / GDPs[i]);
            else
                gdpGrowth.push_back(0.0);
        }
        for (size_t i = 0; i < gdpGrowth.size(); i++) {
            cout << gdpGrowth[i];
            if (i != gdpGrowth.size() - 1) cout << ", ";
        }
        cout << "]\n";
        
        cout << "GDP per Capita Values = [";
        for (size_t i = 0; i < gdpPerCapitas.size(); i++) {
            cout << gdpPerCapitas[i];
            if (i != gdpPerCapitas.size() - 1) cout << ", ";
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
