#include <iostream>
#include <vector>
#include <memory>
#include <random>
#include <ctime>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <chrono>
#include <cmath>
#include <algorithm>
#include "World.h"
#include "FishingFirm.h"
#include "FisherMan.h"
#include "JobMarket.h"
#include "FishingMarket.h"

using namespace std;

//////////////////////////////////////////////////////////////////////////////////////////

// Structure to hold all simulation parameters.
struct SimulationParameters {
    int testColumn = 1;              // The test column to use (1 = Test1, 2 = Test2, etc.)

    // Basic simulation parameters
    int totalCycles;                // Total simulation cycles (days)
    int totalFisherMen;             // Total number of fishers

    // Derived parameters (given as fractions to be multiplied by totalFisherMen)
    double totalFirms;              // e.g. 0.08 means 8% of the population
    double initialEmployed;         // e.g. 0.90 means 90% of the population
    double totalJobOffers;          // e.g. 0.10 means 10% of the population

    double initialWage;             // Baseline wage / fish price reference
    double cycleScale;              // Number of days per year
    int maxStarvingDays;            // Consecutive days without fish before death
    double annualBirthRate;         // Annual birth rate (e.g., 0.02 for 2%)
    double offeredPriceMean;        // Mean offered price by firms at start
    double perceivedPriceMean;      // Mean perceived price by consumers at start
    double pQuit;                   // Daily probability that an employed fisher quits
    double employeeEfficiency;      // Fish caught per fisher per day

    // Parameters for population distributions
    double ageDistMean;             // Mean for initial age distribution
    double ageDistVariance;         // Variance for age distribution
    double lifetimeDistMean;        // Mean for lifetime distribution
    double lifetimeDistVariance;    // Variance for lifetime distribution
};
//////////////////////////////////////////////////////////////////////////////////////////


// Simple string splitting function (by a given delimiter)
vector<string> split(const string &s, char delimiter) {
    vector<string> tokens;
    string token;
    istringstream tokenStream(s);
    while(getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

// Function to load simulation parameters from a CSV file.
// testColumn is 1-based (1 means use the second token in each row, i.e., Test1).
SimulationParameters loadParameters(const string &filepath) {
    SimulationParameters params; // testColumn defaults to 1 (Test1)
    ifstream file(filepath);
    if (!file.is_open()) {
        cerr << "Could not open parameters file: " << filepath << endl;
        return params;
    }
    string line;
    // Read header line (and ignore it).
    getline(file, line);
    while (getline(file, line)) {
        vector<string> tokens = split(line, ',');
        if (tokens.size() < params.testColumn + 1)
            continue; // Skip if the desired test column doesn't exist.
        string paramName = tokens[0];
        string paramValue = tokens[params.testColumn];  // Use the value from the chosen test column.

        // Assign values according to the parameter name.
        if (paramName == "totalCycles") params.totalCycles = stoi(paramValue);
        else if (paramName == "totalFisherMen") params.totalFisherMen = stoi(paramValue);
        else if (paramName == "totalFirms") params.totalFirms = stod(paramValue);
        else if (paramName == "initialEmployed") params.initialEmployed = stod(paramValue);
        else if (paramName == "totalJobOffers") params.totalJobOffers = stod(paramValue);
        else if (paramName == "initialWage") params.initialWage = stod(paramValue);
        else if (paramName == "cycleScale") params.cycleScale = stod(paramValue);
        else if (paramName == "maxStarvingDays") params.maxStarvingDays = stoi(paramValue);
        else if (paramName == "annualBirthRate") params.annualBirthRate = stod(paramValue);
        else if (paramName == "offeredPriceMean") params.offeredPriceMean = stod(paramValue);
        else if (paramName == "perceivedPriceMean") params.perceivedPriceMean = stod(paramValue);
        else if (paramName == "pQuit") params.pQuit = stod(paramValue);
        else if (paramName == "employeeEfficiency") params.employeeEfficiency = stod(paramValue);
        else if (paramName == "ageDistMean") params.ageDistMean = stod(paramValue);
        else if (paramName == "ageDistVariance") params.ageDistVariance = stod(paramValue);
        else if (paramName == "lifetimeDistMean") params.lifetimeDistMean = stod(paramValue);
        else if (paramName == "lifetimeDistVariance") params.lifetimeDistVariance = stod(paramValue);
    }
    // Recompute derived parameters as integers.
    params.totalFirms = static_cast<int>(params.totalFirms * params.totalFisherMen);
    if (params.totalFirms < 1)
        params.totalFirms = 1;
    params.initialEmployed = static_cast<int>(params.initialEmployed * params.totalFisherMen);
    params.totalJobOffers = static_cast<int>(params.totalJobOffers * params.totalFisherMen);
    return params;
}

//////////////////////////////////////////////////////////////////////////////////////////
// The Simulation class remains largely the same.
class Simulation {
private:
    SimulationParameters params;
    shared_ptr<JobMarket> jobMarket;
    shared_ptr<FishingMarket> fishingMarket;
    World world;
    vector<shared_ptr<FishingFirm>> firms;
    vector<shared_ptr<FisherMan>> fishers;
    default_random_engine generator;
    std::normal_distribution<double> firmFundsDist; // e.g., N(100, 20)
    double currentOfferMean;
    double currentPerceivedMean;
    std::normal_distribution<double> firmPriceDist;    // e.g., N(offeredPriceMean, 0.5)
    std::normal_distribution<double> consumerPriceDist;  // e.g., N(perceivedPriceMean, 0.8)
    std::normal_distribution<double> fisherLifetimeDist; // N(lifetimeDistMean, lifetimeDistVariance)
    std::normal_distribution<double> fisherAgeDist;      // N(ageDistMean, ageDistVariance)
    std::uniform_int_distribution<int> goodsQuantityDist; // Uniform between 1 and 3

public:
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
        // Compute initialStock as integer division: totalFisherMen / totalFirms.
        int initialStock = params.totalFisherMen / params.totalFirms;
        // Compute initial employees per firm as integer: initialEmployed / totalFirms.
        int initialEmployeesPerFirm = params.initialEmployed / params.totalFirms;

        for (int id = 100; id < 100 + params.totalFirms; id++) {
            double funds = firmFundsDist(generator);
            int stock = initialStock;
            int lifetime = 100000000; // A very large number for firm lifetime.
            double salesEff = params.employeeEfficiency;
            auto firm = make_shared<FishingFirm>(id, funds, lifetime, initialEmployeesPerFirm, stock, salesEff);
            double price = firmPriceDist(generator);
            firm->setPriceLevel(price);
            firms.push_back(firm);
            world.addFirm(firm);
        }

        // Create employed fishers.
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
        
        // Create unemployed fishers.
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

    void run() {
        vector<double> GDPs, unemploymentRates, inflations, gdpPerCapitas, cyclyGDPs;
        vector<int> populations;
        ofstream summaryFile("/Users/avass/Documents/1SSE/Code/FishingVillage/data/economicdatas.csv");
        if (summaryFile.is_open())
            summaryFile << "Cycle,Year,DailyGDP,CyclyGDP,Population,GDPperCapita,Unemployment,Inflation\n";
        else
            cerr << "Error: Unable to open file for writing summary data.\n";

        double annualGDPAccumulator = 0.0;
        for (int day = 0; day < params.totalCycles; day++) {
            int cycle = day + 1;
            double currentYear = cycle / params.cycleScale;
            world.simulateCycle(generator, firmPriceDist, goodsQuantityDist, consumerPriceDist);
            double updatedFishPrice = fishingMarket->getClearingFishPrice();
            jobMarket->setCurrentFishPrice(updatedFishPrice);
            jobMarket->clearMarket(generator);
            int vacanciesPerFirm = std::max(1, static_cast<int>(params.totalJobOffers / params.totalFirms));
            for (auto &firm : firms) {
                JobPosting posting = firm->generateJobPosting("fishing", 1, 1, 1);
                posting.vacancies = vacanciesPerFirm;
                jobMarket->submitJobPosting(posting);
            }
            for (auto &fisher : fishers) {
                if (fisher->isEmployed() && (static_cast<double>(rand()) / RAND_MAX) < params.pQuit)
                    fisher->setEmployed(false);
            }
            int totalFishers = world.getTotalFishers();
            populations.push_back(totalFishers);
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
            double aggSupply = fishingMarket->getAggregateSupply();
            double aggDemand = fishingMarket->getAggregateDemand();
            double ratio = (aggSupply > 0) ? aggDemand / aggSupply : 1.0;
            double factor = (ratio > 1.0) ? std::normal_distribution<double>(1.025, 0.005)(generator)
                                          : (ratio < 1.0) ? std::normal_distribution<double>(0.975, 0.005)(generator)
                                                          : 1.0;
            currentOfferMean *= factor;
            currentPerceivedMean *= factor;
            firmPriceDist.param(std::normal_distribution<double>::param_type(currentOfferMean, 0.5));
            consumerPriceDist.param(std::normal_distribution<double>::param_type(currentPerceivedMean, 0.8));
            if (cycle % static_cast<int>(params.cycleScale) == 0 || day == params.totalCycles - 1) {
                cyclyGDPs.push_back(annualGDPAccumulator);
                annualGDPAccumulator = 0.0;
            }
            if (summaryFile.is_open())
                summaryFile << cycle << "," << currentYear << "," << dailyGDP << ","
                            << ((cycle % static_cast<int>(params.cycleScale) == 0 || day == params.totalCycles - 1)
                                ? cyclyGDPs.back() : 0.0) << "," << totalFishers << ","
                            << perCapita << "," << dailyUnemploymentRate << ","
                            << inflRate * 100 << "\n";
        }
        if (summaryFile.is_open())
            summaryFile.close();
    }
};

//////////////////////////////////////////////////////////////////////////////////////////

int main() {
    SimulationParameters params = loadParameters("/Users/avass/Documents/1SSE/Code/FishingVillage/data/initialparameters.csv");
    Simulation sim(params);
    cout << " BEGIN program ... " << endl;
    cout << "   days to simulate = " << params.totalCycles << endl;
    cout << "   initial number of fishers = " << params.totalFisherMen << endl;
    cout << "   calculated number of firms = " << params.totalFirms << endl;
    cout << " -------------------------- " << endl;
    
    auto start = chrono::high_resolution_clock::now();
    sim.run();
    // Optionally, run visualization (e.g., via a Python script).
    cout << "  ... END program  " << endl;
    cout << " -------------------------- " << endl;
    
    auto stop = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = stop - start;
    cout << "elapsed time: " << elapsed.count() << " seconds" << endl;
    
    return 0;
}
