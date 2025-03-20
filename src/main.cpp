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
// Structure to hold all simulation parameters, grouped logically.
struct SimulationParameters {
    // 1. Test Configuration & Global Settings
    int testColumn = 1;              // The test column to use (1 = Test1, 2 = Test2, etc.)
    int totalCycles;                 // Total simulation cycles (days)
    double cycleScale;               // Number of days per year

    // 2. Population & Demographic Management
    int totalFisherMen;              // Total number of fishers
    double annualBirthRate;          // Annual birth rate (e.g., 0.02 for 2%)
    int maxStarvingDays;             // Consecutive days without fish before death
    double ageDistMean;              // Mean for initial age distribution
    double ageDistVariance;          // Variance for age distribution
    double lifetimeDistMean;         // Mean for lifetime distribution
    double lifetimeDistVariance;     // Variance for lifetime distribution

    // 3. Derived Population Fractions
    double totalFirms;               // e.g. 0.08 means 8% of the population (fraction)
    double initialEmployed;          // e.g. 0.90 means 90% of the population (fraction)
    double totalJobOffers;           // e.g. 0.10 means 10% of the population (fraction)

    // 4. Economic Policy / Market Parameters
    double initialWage;              // Baseline wage / fish price reference
    double offeredPriceMean;         // Mean offered price by firms at start
    double perceivedPriceMean;       // Mean perceived price by consumers at start
    double pQuit;                    // Daily probability that an employed fisher quits
    double employeeEfficiency;       // Fish caught per fisher per day

    // 5. Inflation Adjustment Parameters
    double meanAugmentationInflat;     // Mean factor when demand > supply (e.g., 1.025)
    double varianceAugmentationInflat;  // Variance for augmentation factor (e.g., 0.005)
    double meanDiminutionInflat;       // Mean factor when supply > demand (e.g., 0.975)
    double varianceDiminutionInflat;    // Variance for diminution factor (e.g., 0.005)
};

///////////////////////////////////////////////////////////////////////////////
// Function to parse simulation parameters from a file.
// The file should contain the 21 parameters in order, separated by whitespace or newlines.
SimulationParameters parseParametersFromFile(const string &filepath) {
    SimulationParameters params;
    ifstream file(filepath);
    if (!file) {
        cerr << "Error: Cannot open file " << filepath << endl;
        exit(1);
    }
    file >> params.totalCycles >> params.cycleScale >> params.totalFisherMen >> params.annualBirthRate
         >> params.maxStarvingDays >> params.ageDistMean >> params.ageDistVariance >> params.lifetimeDistMean
         >> params.lifetimeDistVariance >> params.totalFirms >> params.initialEmployed >> params.totalJobOffers
         >> params.initialWage >> params.offeredPriceMean >> params.perceivedPriceMean >> params.pQuit
         >> params.employeeEfficiency >> params.meanAugmentationInflat >> params.varianceAugmentationInflat
         >> params.meanDiminutionInflat >> params.varianceDiminutionInflat;

    // Convert fractional parameters into counts.
    params.totalFirms = static_cast<int>(params.totalFirms * params.totalFisherMen);
    if (params.totalFirms < 1)
        params.totalFirms = 1;
    params.initialEmployed = static_cast<int>(params.initialEmployed * params.totalFisherMen);
    params.totalJobOffers = static_cast<int>(params.totalJobOffers * params.totalFisherMen);
    return params;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Simulation class encapsulating the simulation logic.
class Simulation {
private:
    SimulationParameters params;
    // Our firms vector is now stored here.
    std::vector<std::shared_ptr<FishingFirm>> firms;
    std::vector<std::shared_ptr<FisherMan>> fishers;
    std::shared_ptr<JobMarket> jobMarket;
    std::shared_ptr<FishingMarket> fishingMarket;
    World world;
    default_random_engine generator;
    std::normal_distribution<double> firmFundsDist; // e.g., N(100, 20)
    double currentOfferMean;
    double currentPerceivedMean;
    std::normal_distribution<double> firmPriceDist;    // e.g., N(offeredPriceMean, 0.5)
    std::normal_distribution<double> consumerPriceDist;  // e.g., N(perceivedPriceMean, 0.8)
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
          goodsQuantityDist(1, 3)
    {
        // Build the firms vector.
        int initialStock = params.totalFisherMen / params.totalFirms;
        int initialEmployeesPerFirm = params.initialEmployed / params.totalFirms;
        for (int id = 100; id < 100 + params.totalFirms; id++) {
            double funds = firmFundsDist(generator);
            int stock = initialStock;
            int lifetime = 100000000; // Firm lifetime (days)
            double salesEff = params.employeeEfficiency;
            auto firm = make_shared<FishingFirm>(id, funds, lifetime, initialEmployeesPerFirm, stock, salesEff);
            double price = firmPriceDist(generator);
            firm->setPriceLevel(price);
            firms.push_back(firm);
            // Optionally, also add firm to World if needed.
            // world.addFirm(firm);
        }
        // After constructing firms, pass the vector to World.
        world.setFirms(firms);

        // Build the fishers vector.
        for (int id = 0; id < params.initialEmployed; id++) {
            double age = 0;
            double lifetime = 365 * 60; // for example, 60 years.
            auto fisher = make_shared<FisherMan>(
                id, 0.0, lifetime, 0.0, 0.0, 1.0, 1.0, true,
                params.initialWage, 0.0, "fishing", 1, 1, 1
            );
            fishers.push_back(fisher);
        }
        // Add unemployed fishers to complete the population.
        for (int id = params.initialEmployed; id < params.totalFisherMen; id++) {
            double age = 0;
            double lifetime = 365 * 60;
            auto fisher = make_shared<FisherMan>(
                id, 0.0, lifetime, 0.0, 0.0, 1.0, 1.0, false,
                0.0, 0.0, "fishing", 1, 1, 1
            );
            fishers.push_back(fisher);
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
        double prevOfferMean = currentOfferMean;

        // Declare consumerPriceDist locally (non-const) for use in orders.
        std::normal_distribution<double> localConsumerPriceDist(params.perceivedPriceMean, 0.8);

        for (int day = 0; day < params.totalCycles; day++) {
            int cycle = day + 1;
            double currentYear = cycle / params.cycleScale;

            // Refresh supply via World.
            world.simulateCycle(generator, firmPriceDist, goodsQuantityDist, localConsumerPriceDist);

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

            // Retrieve market aggregates and adjust price means.
            double aggSupply = fishingMarket->getAggregateSupply();
            double aggDemand = fishingMarket->getAggregateDemand();
            double ratio = (aggSupply > 0) ? aggDemand / aggSupply : 1.0;
            double factor = 1.0;
            if (ratio > 1.0) {
                std::normal_distribution<double> adjustDist(params.meanAugmentationInflat, params.varianceAugmentationInflat);
                factor = adjustDist(generator);
            } else if (ratio < 1.0) {
                std::normal_distribution<double> adjustDist(params.meanDiminutionInflat, params.varianceDiminutionInflat);
                factor = adjustDist(generator);
            }
            double oldOfferMean = currentOfferMean;
            currentOfferMean *= factor;
            currentPerceivedMean *= factor;
            firmPriceDist.param(std::normal_distribution<double>::param_type(currentOfferMean, 0.5));
            localConsumerPriceDist.param(std::normal_distribution<double>::param_type(currentPerceivedMean, 0.8));

            double inflRate = (oldOfferMean > 0) ? (currentOfferMean - oldOfferMean) / oldOfferMean : 0.0;
            inflations.push_back(inflRate);

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

        double finalAggSupply = fishingMarket->getAggregateSupply();
        double finalAggDemand = fishingMarket->getAggregateDemand();
        double finalRatio = (finalAggSupply > 0) ? finalAggDemand / finalAggSupply : 1.0;
        cout << "Final Aggregate Demand/Supply Ratio = " << finalRatio << endl;
    }
};


//////////////////////////////////////////////////////////////////////////////////////////
// Main function.
int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <parameters_file>" << endl;
        return 1;
    }
    SimulationParameters params = parseParametersFromFile(argv[1]);
    Simulation sim(params);

    cout << "BEGIN program ..." << endl;
    cout << "Days to simulate: " << params.totalCycles << endl;
    cout << "Initial number of fishers: " << params.totalFisherMen << endl;
    cout << "Calculated number of firms: " << params.totalFirms << endl;
    cout << "--------------------------" << endl;

    auto start = chrono::high_resolution_clock::now();
    sim.run();
    cout << "... END program" << endl;
    cout << "--------------------------" << endl;

    auto stop = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = stop - start;
    cout << "Elapsed time: " << elapsed.count() << " seconds" << endl;

    return 0;
}