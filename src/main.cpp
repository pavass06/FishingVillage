#include <iostream>
#include <vector>
#include <memory>
#include <random>
#include <ctime>
#include <cstdlib> // Required for system()
#include <fstream> // For file output
#include <chrono>
#include <cmath>
#include "World.h"
#include "FishingFirm.h"
#include "FisherMan.h"
#include "JobMarket.h"
#include "FishingMarket.h"

using namespace std;

// Simulation parameters structure
struct SimulationParameters {
    // Basic simulation parameters
    int totalCycles = 300;          // Total simulation cycles (days)
    int totalFisherMen = 100;       // Total number of fishers

    // Derived parameters (computed as a percentage of totalFisherMen)
    double totalFirms = 0.08;                 // 8% of the population (at least 1 firm)
    double initialEmployed = 0.90;            // 90% of the population
    double totalJobOffers = 0.10;             // 10% of the population

    double initialWage = 5.0;       // Baseline wage / fish price reference
    double cycleScale = 365;        // Number of days per year
    int maxStarvingDays = 5;        // Number of consecutive days without fish before death
    double annualBirthRate = 0.02;  // Annual birth rate (e.g., 2%)
    double offeredPriceMean = 5.1;  // Mean offered price by firms at start
    double perceivedPriceMean = 5.0;// Mean perceived price by consumers at start
    double pQuit = 0.10;            // Daily probability that an employed fisher quits
    double employeeEfficiency = 2.0; // How many fish a single fisher catches per day


    // Parameters for population distributions
    double ageDistMean = 30.0;      // Mean for initial age distribution
    double ageDistVariance = 20.0;  // Variance for age distribution
    double lifetimeDistMean = 60.0; // Mean for lifetime distribution
    double lifetimeDistVariance = 5.0;// Variance for lifetime distribution

    // Constructor computes derived parameters as integer percentages of totalFisherMen
    SimulationParameters() {
        totalFirms = static_cast<int>(totalFirms * totalFisherMen);
        if(totalFirms < 1) totalFirms = 1;
        initialEmployed = static_cast<int>(initialEmployed * totalFisherMen);
        totalJobOffers = static_cast<int>(totalJobOffers * totalFisherMen);
    }
};

// Simulation class encapsulating the simulation logic
class Simulation {
private:
    SimulationParameters params;
    // Instantiate markets and world
    shared_ptr<JobMarket> jobMarket;
    shared_ptr<FishingMarket> fishingMarket;
    World world;
    vector<shared_ptr<FishingFirm>> firms;
    vector<shared_ptr<FisherMan>> fishers;

    // Random number generator
    default_random_engine generator;

    // Normal distributions for firm funds and stock (unused now for stock)
    std::normal_distribution<double> firmFundsDist; // N(100, 20)
    // The initial stock is now computed by a rule instead of a random distribution:
    // std::normal_distribution<double> firmStockDist; // Removed for initial stock

    // Evolving means for offered and perceived prices
    double currentOfferMean;     // Evolving mean for firm's offered price
    double currentPerceivedMean; // Evolving mean for consumer's perceived price

    // Distributions for firm offered price and consumer perceived price
    std::normal_distribution<double> firmPriceDist;    // Initially N(offeredPriceMean, 0.5)
    std::normal_distribution<double> consumerPriceDist;  // Initially N(perceivedPriceMean, 0.8)

    // Distributions for fisher lifetime and age
    std::normal_distribution<double> fisherLifetimeDist; // N(lifetimeDistMean, lifetimeDistVariance)
    std::normal_distribution<double> fisherAgeDist;      // N(ageDistMean, ageDistVariance)
    std::uniform_int_distribution<int> goodsQuantityDist; // Uniform between 1 and 3

public:
    // Constructor: initialize simulation parameters, markets, and distributions
    Simulation(const SimulationParameters &p)
        : params(p),
          jobMarket(make_shared<JobMarket>(p.initialWage, p.perceivedPriceMean, 1)),
          fishingMarket(make_shared<FishingMarket>(p.perceivedPriceMean)),
          world(p.totalCycles, p.annualBirthRate, jobMarket, fishingMarket, p.maxStarvingDays),
          generator(static_cast<unsigned int>(time(0))),
          firmFundsDist(100.0, 20.0),
          currentOfferMean(p.offeredPriceMean),
          currentPerceivedMean(p.perceivedPriceMean),
          firmPriceDist(p.offeredPriceMean, 0.5),
          consumerPriceDist(p.perceivedPriceMean, 0.8),
          fisherLifetimeDist(p.lifetimeDistMean, p.lifetimeDistVariance),
          fisherAgeDist(p.ageDistMean, p.ageDistVariance),
          goodsQuantityDist(1, 3)
    {
        // Initialize FishingFirms with initialStock computed as population/numberFirms.
        // Ensuring the stock is an integer.
        int initialStock = params.totalFisherMen / params.totalFirms;
        // Compute initial employees per firm as the integer part of (initialEmployed / totalFirms)
        int initialEmployeesPerFirm = params.initialEmployed / params.totalFirms;
        for (int id = 100; id < 100 + params.totalFirms; id++) {
            double funds = firmFundsDist(generator);
            int stock = initialStock; // Updated rule: initialStock = population / numberFirms
            int lifetime = 100000000; // Firm lifetime (days)
            
            // Use the parameter for employee efficiency and the computed initial employees per firm.
            double salesEff = params.employeeEfficiency;
            // Replace the hardcoded "18" with the computed number of initial employees per firm:
            auto firm = make_shared<FishingFirm>(id, funds, lifetime, initialEmployeesPerFirm, stock, salesEff);
            double price = firmPriceDist(generator);
            firm->setPriceLevel(price);
            firms.push_back(firm);
            world.addFirm(firm);
        }

        // Initialize employed FisherMen (using 90% of totalFisherMen)
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
        
        // Initialize unemployed FisherMen (remaining population)
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

        // Open a CSV file for writing the simulation summary.
        ofstream summaryFile("/Users/avass/Documents/1SSE/Code/FishingVillage/data/economicdatas.csv");
        if (summaryFile.is_open()) {
            summaryFile << "Cycle,Year,DailyGDP,CyclyGDP,Population,GDPperCapita,Unemployment,Inflation\n";
        } else {
            cerr << "Error: Unable to open file for writing summary data.\n";
        }
        
        double annualGDPAccumulator = 0.0;
        
        // Simulation loop (each cycle represents one day).
        for (int day = 0; day < params.totalCycles; day++) {
            int cycle = day + 1; // Cycle number (starting at 1)
            double currentYear = cycle / params.cycleScale;  // Convert cycle to years

#if verbose==1
            cout << "===== Day " << cycle << " (Year " << currentYear << ") =====" << endl;
#endif
            // Run one simulation cycle.
            world.simulateCycle(generator, firmPriceDist, goodsQuantityDist, consumerPriceDist);
            
            // ---- Job Market Update and Turnover ----
            double updatedFishPrice = fishingMarket->getClearingFishPrice();
            jobMarket->setCurrentFishPrice(updatedFishPrice);
            jobMarket->clearMarket(generator);  // Recalculate clearing wage
            
            // --- JOB POSTING: Limit total job offers to params.totalJobOffers ---
            // Calculate vacancies per firm (ensuring an integer result):
            int vacanciesPerFirm = std::max(1, static_cast<int>(params.totalJobOffers / params.totalFirms));
            // For each firm, generate a job posting with the computed vacancies.
            for (auto &firm : firms) {
                JobPosting posting = firm->generateJobPosting("fishing", 1, 1, 1);
                posting.vacancies = vacanciesPerFirm;
                jobMarket->submitJobPosting(posting);
            }
            
            // Turnover: each employed fisher quits with probability pQuit.
            for (auto &fisher : fishers) {
                if (fisher->isEmployed()) {
                    double r = static_cast<double>(rand()) / RAND_MAX;
                    if (r < params.pQuit) {
                        fisher->setEmployed(false);
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

    cout << " BEGIN program ... " << endl;
    cout << "   days to simulate = " << params.totalCycles << endl;
    cout << "   initial number of fishers = " << params.totalFisherMen << endl;
    cout << "   calculated number of firms = " << params.totalFirms << endl;
    cout << " -------------------------- " << endl;
    auto start = chrono::high_resolution_clock::now();

    sim.run();
    // Optionally, call the Python script for visualization:
    // system("/Users/avass/anaconda3/bin/python /Users/avass/Documents/1SSE/Code/FishingVillage/python/display.py");
    
    cout << "  ... END program  " << endl;
    cout << " -------------------------- " << endl;

    auto stop = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = stop - start;
    cout << "elapsed time: " << elapsed.count() << " seconds" << endl;

    return 0;
}
