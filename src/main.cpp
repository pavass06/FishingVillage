#include <iostream>
#include <vector>
#include <memory>
#include <random>
#include <ctime>
#include <cstdlib> // Required for system()
#include <fstream> // For file output
#include "World.h"
#include "FishingFirm.h"
#include "FisherMan.h"
#include "JobMarket.h"
#include "FishingMarket.h"

using namespace std;

// Simulation parameters structure
struct SimulationParameters {
    int totalCycles = 10000;          // Total simulation cycles (days)
    int totalFisherMen = 100;       // Total number of fishermen
    int totalFirms = 5;             // Total number of fishing firms
    int initialEmployed = 90;       // Number of employed fishermen at start
    double initialWage = 5.0;       // Base wage (here used as fish price input)
    
    
    // New parameters:
    double cycleScale = 365;        // Number of cycles per year (for time conversion)
    int maxStarvingDays = 10;        // Number of days a fisherman can survive without eating
    double annualBirthRate = 0.02;          // Annual birth rate (e.g., 2%)
    double offeredPriceMean = 5.1;          // Mean of the offered price distribution
    double perceivedPriceMean = 5.0;        // Mean of the perceived price distribution
    int totalJobOffers = 10;                // Total job offers available per day (across all companies)
    double pQuit = 0.10;                    // 10% chance per day that an employed fisherman quits
};

// Simulation class encapsulating the simulation logic
class Simulation {
private:
    SimulationParameters params;
    // Instantiate JobMarket with: initial wage, fish price = 5.0, and mean fish order = 1.
    shared_ptr<JobMarket> jobMarket;
    shared_ptr<FishingMarket> fishingMarket;
    World world;
    vector<shared_ptr<FishingFirm>> firms;
    vector<shared_ptr<FisherMan>> fishers;

    // Random number generator
    default_random_engine generator;

    // Normal distributions for firm funds and stock
    std::normal_distribution<double> firmFundsDist; // N(100, 20)
    std::normal_distribution<double> firmStockDist;   // N(50, 10)

    // Evolving means for offered and perceived prices
    double currentOfferMean;     // Evolving mean for firm's offered price
    double currentPerceivedMean; // Evolving mean for consumer's perceived price

    // Distributions for firm offered price and consumer perceived price
    std::normal_distribution<double> firmPriceDist;    // Initially N(offeredPriceMean, 0.5)
    std::normal_distribution<double> consumerPriceDist;  // Initially N(perceivedPriceMean, 0.8)

    // Other distributions
    std::normal_distribution<double> fisherLifetimeDist; // N(60, 5) in years
    std::normal_distribution<double> fisherAgeDist;      // N(30, 10)
    std::uniform_int_distribution<int> goodsQuantityDist; // Uniform between 1 and 3

public:
    // Constructor: initialize simulation parameters, markets, and distributions
    Simulation(const SimulationParameters &p)
        : params(p),
          jobMarket(make_shared<JobMarket>(p.initialWage, 5.0, 1)),
          fishingMarket(make_shared<FishingMarket>(5.0)),
          world(params.totalCycles, params.annualBirthRate, jobMarket, fishingMarket, params.maxStarvingDays),
          generator(static_cast<unsigned int>(time(0))),
          firmFundsDist(100.0, 20.0),
          firmStockDist(50.0, 10.0),
          currentOfferMean(params.offeredPriceMean),
          currentPerceivedMean(params.perceivedPriceMean),
          firmPriceDist(params.offeredPriceMean, 0.5),
          consumerPriceDist(params.perceivedPriceMean, 0.8),
          fisherLifetimeDist(60.0, 5.0),
          fisherAgeDist(30.0, 10.0),
          goodsQuantityDist(1, 3)
    {
        // Initialize FishingFirms
        for (int id = 100; id < 100 + params.totalFirms; id++) {
            double funds = firmFundsDist(generator);
            double stock = std::floor(firmStockDist(generator)); // Floor the stock to ensure whole fish.
            int lifetime = 100000; // Firm lifetime (days)
            double salesEff = 2.0;
            double jobMult = 0.05;
            double price = firmPriceDist(generator);
            
            auto firm = make_shared<FishingFirm>(id, funds, lifetime, 18, stock, salesEff);
            firm->setPriceLevel(price);
            firms.push_back(firm);
            world.addFirm(firm);
        }

        // Initialize employed FisherMen
        for (int id = 0; id < params.initialEmployed; id++) {
            double age = fisherAgeDist(generator) * 365;
            double lifetime = fisherLifetimeDist(generator) * 365;
            
            auto fisher = make_shared<FisherMan>(
                id, 0.0, lifetime, 0.0, 0.0, 1.0, 1.0, true,
                params.initialWage, 0.0, "fishing", 1, 1, 1
            );
            fishers.push_back(fisher);
            world.addFisherMan(fisher);
        }
        
        // Initialize unemployed FisherMen
        for (int id = params.initialEmployed; id < params.totalFisherMen; id++) {
            double age = fisherAgeDist(generator) * 365;
            double lifetime = fisherLifetimeDist(generator) * 365;
            
            auto fisher = make_shared<FisherMan>(
                id, 0.0, lifetime, 0.0, 0.0, 1.0, 1.0, false,
                0.0, 0.0, "fishing", 1, 1, 1
            );
            fishers.push_back(fisher);
            world.addFisherMan(fisher);
        }
    }

    // Run the simulation cycles.
    void run() {
        vector<double> GDPs;
        vector<double> unemploymentRates;
        vector<double> inflations;
        vector<double> gdpPerCapitas;
        vector<int> populations;
        vector<double> cyclyGDPs;

        ofstream summaryFile("/Users/avass/Documents/1SSE/Code/FishingVillage/data/simulation_summary.csv");
        if (summaryFile.is_open()) {
            summaryFile << "Cycle,Year,DailyGDP,CyclyGDP,Population,GDPperCapita,Unemployment,Inflation\n";
        } else {
            cerr << "Error: Unable to open file for writing summary data.\n";
        }
        
        double annualGDPAccumulator = 0.0;
        
        // Simulation loop (each cycle represents one day).
        for (int day = 0; day < params.totalCycles; day++) {
            int cycle = day + 1;
            double currentYear = cycle / params.cycleScale;
            
            cout << "===== Day " << cycle << " (Year " << currentYear << ") =====" << endl;
            
            // Run one simulation cycle.
            world.simulateCycle(generator, firmPriceDist, goodsQuantityDist, consumerPriceDist);
            
            // ---- Job Market Update and Turnover ----
            double updatedFishPrice = fishingMarket->getClearingFishPrice();
            jobMarket->setCurrentFishPrice(updatedFishPrice);
            jobMarket->clearMarket(generator);  // Recalculate clearing wage
            
            // --- JOB POSTING: Limit total job offers to params.totalJobOffers ---
            // Calculate vacancies per firm:
            int vacanciesPerFirm = std::max(1, params.totalJobOffers / params.totalFirms);
            // Overwrite each firm's generated posting to use vacanciesPerFirm.
            for (auto &firm : firms) {
                // Generate a posting using the firm's method.
                JobPosting posting = firm->generateJobPosting("fishing", 1, 1, 1);
                posting.vacancies = vacanciesPerFirm; // Force each firm to offer vacanciesPerFirm jobs.
                jobMarket->submitJobPosting(posting);
            }
            
            // Turnover: each employed fisherman quits with probability pQuit.
            for (auto &fisher : fishers) {
                if (fisher->isEmployed()) {
                    double r = static_cast<double>(rand()) / RAND_MAX;
                    if (r < params.pQuit) {
                        fisher->setEmployed(false);
                        // Optionally: cout << "Fisher " << fisher->getID() << " quit his job." << endl;
                    }
                }
            }
            
            // Retrieve current population.
            int totalFishers = world.getTotalFishers();
            populations.push_back(totalFishers);
            
            // Retrieve daily GDP.
            double dailyGDP = world.getGDP();
            GDPs.push_back(dailyGDP);
            annualGDPAccumulator += dailyGDP;
            
            double perCapita = (totalFishers > 0) ? dailyGDP / totalFishers : 0.0;
            gdpPerCapitas.push_back(perCapita);
            
            int unemployedFishers = world.getUnemployedFishers();
            double dailyUnemploymentRate = (totalFishers > 0) ?
                (static_cast<double>(unemployedFishers) / totalFishers) * 100.0 : 0.0;
            unemploymentRates.push_back(dailyUnemploymentRate);
            
            static double prevFishPrice = updatedFishPrice;
            double currFishPrice = fishingMarket->getClearingFishPrice();
            double inflRate = (prevFishPrice > 0) ? (currFishPrice - prevFishPrice) / prevFishPrice : 0.0;
            inflations.push_back(inflRate);
            prevFishPrice = currFishPrice;
            
            // Update evolving price means.
            double aggSupply = fishingMarket->getAggregateSupply();
            double aggDemand = fishingMarket->getAggregateDemand();
            double ratio = (aggSupply > 0) ? aggDemand / aggSupply : 1.0;
            double factor = 1.0;
            if (ratio > 1.0) {
                std::normal_distribution<double> adjustDist(1.025, 0.005);
                factor = adjustDist(generator);
            } else if (ratio < 1.0) {
                std::normal_distribution<double> adjustDist(0.975, 0.005);
                factor = adjustDist(generator);
            }
            currentOfferMean *= factor;
            currentPerceivedMean *= factor;
            firmPriceDist.param(std::normal_distribution<double>::param_type(currentOfferMean, 0.5));
            consumerPriceDist.param(std::normal_distribution<double>::param_type(currentPerceivedMean, 0.8));
            
            if (cycle % static_cast<int>(params.cycleScale) == 0 || day == params.totalCycles - 1) {
                cyclyGDPs.push_back(annualGDPAccumulator);
                annualGDPAccumulator = 0.0;
            }
            
            if (summaryFile.is_open()) {
                double cyclyGDPOutput = (cycle % static_cast<int>(params.cycleScale) == 0 || day == params.totalCycles - 1)
                                        ? cyclyGDPs.back() : 0.0;
                summaryFile << cycle << ","
                            << currentYear << ","
                            << dailyGDP << ","
                            << cyclyGDPOutput << ","
                            << totalFishers << ","
                            << perCapita << ","
                            << dailyUnemploymentRate << ","
                            << inflRate * 100
                            << "\n";
            }
        }
        
        if (summaryFile.is_open())
            summaryFile.close();
    }
};

int main() {
    SimulationParameters params;
    Simulation sim(params);
    sim.run();
    // Run the Python script for visualization.
    system("/Users/avass/anaconda3/bin/python /Users/avass/Documents/1SSE/Code/FishingVillage/python/display.py");
    return 0;
}
