#include <iostream>
#include <chrono>
#include <fstream>
#include <random>
#include <cstdlib>
#include "World.h"
#include "FishingFirm.h"
#include "FisherMan.h"
#include "JobMarket.h"
#include "FishingMarket.h"
#include "SimulationParameters.h"  // Contains the definition of SimulationParameters and parseParametersFromFile

using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <parameters_file>" << endl;
        return 1;
    }
    // Parse simulation parameters from a file.
     SimulationParameters params = parseParametersFromFile(argv[1]);

    // Create shared market objects.
    auto jobMarket = make_shared<JobMarket>(params.initialWage, params.perceivedPriceMean, 1);
    auto fishingMarket = make_shared<FishingMarket>(params.perceivedPriceMean);

    // Create the World object.
    World world(params.totalCycles,
                params.annualBirthRate,
                jobMarket,
                fishingMarket,
                params.maxStarvingDays,
                params.offeredPriceMean,
                params.perceivedPriceMean,
                params.meanAugmentationInflat,
                params.varianceAugmentationInflat,
                params.meanDiminutionInflat,
                params.varianceDiminutionInflat);

    // Build the vector of FishingFirm objects.
    vector<shared_ptr<FishingFirm>> firms;
    int initialStock = params.totalFisherMen / params.totalFirms;
    int initialEmployeesPerFirm = params.initialEmployed / params.totalFirms;
    default_random_engine generator(static_cast<unsigned int>(time(0)));
    normal_distribution<double> firmFundsDist(100.0, 20.0);
    normal_distribution<double> firmPriceDist(params.offeredPriceMean, 0.5);
    for (int id = 100; id < 100 + params.totalFirms; id++) {
        double funds = firmFundsDist(generator);
        int lifetime = 100000000; // Firm lifetime (days)
        auto firm = make_shared<FishingFirm>(id, funds, lifetime, initialEmployeesPerFirm, initialStock, params.employeeEfficiency);
        double price = firmPriceDist(generator);
        firm->setPriceLevel(price);
        firms.push_back(firm);
    }
    world.setFirms(firms);

    // Build and add FisherMan objects to the World.
    for (int id = 0; id < params.initialEmployed; id++) {
        double lifetime = 365 * 60; // e.g., 60 years.
        auto fisher = make_shared<FisherMan>(
            id, 0.0, lifetime, 0.0, 0.0, 1.0, 1.0, true,
            params.initialWage, 0.0, "fishing", 1, 1, 1
        );
        world.addFisherMan(fisher);
    }
    for (int id = params.initialEmployed; id < params.totalFisherMen; id++) {
        double lifetime = 365 * 60;
        auto fisher = make_shared<FisherMan>(
            id, 0.0, lifetime, 0.0, 0.0, 1.0, 1.0, false,
            0.0, 0.0, "fishing", 1, 1, 1
        );
        world.addFisherMan(fisher);
    }

    cout << "BEGIN program ..." << endl;
    cout << "Days to simulate: " << params.totalCycles << endl;
    cout << "Initial number of fishers: " << params.totalFisherMen << endl;
    cout << "Calculated number of firms: " << params.totalFirms << endl;
    cout << "--------------------------" << endl;

    // Open the output file in the specified directory.
    ofstream summaryFile("/Users/avass/Documents/1SSE/Code/FishingVillage/data/output/economicdatas.csv");
    if (!summaryFile.is_open()) {
        cerr << "Error: Unable to open output file." << endl;
        return 1;
    }
    // Write header line.
    summaryFile << "Cycle,Year,DailyGDP,CyclyGDP,Population,GDPperCapita,Unemployment,Inflation\n";

    auto start = chrono::high_resolution_clock::now();
    normal_distribution<double> localConsumerPriceDist(params.perceivedPriceMean, 0.8);
    uniform_int_distribution<int> goodsQuantityDist(1, 3);

    // Simulation loop: process each cycle/day.
    for (int day = 0; day < params.totalCycles; day++) {
        // Process one simulation cycle.
        world.simulateCycle(generator, firmPriceDist, goodsQuantityDist, localConsumerPriceDist);

        int cycle = day + 1;
        double currentYear = cycle / params.cycleScale;  
        double dailyGDP = world.getGDP();                  
        int totalFishers = world.getTotalFishers();        
        int unemployed = world.getUnemployedFishers();       
        double unemploymentRate = world.getUnemploymentRate();  
        double perCapita = (totalFishers > 0) ? (dailyGDP / totalFishers) : 0.0;
        double cyclyGDP = dailyGDP / params.cycleScale;   
        double inflation = world.getInflation(cycle);  
        
        // Write a summary line for this cycle to the CSV file.
        summaryFile << cycle << "," << currentYear << "," << dailyGDP << ","
                    << cyclyGDP << "," << totalFishers << "," << perCapita << ","
                    << unemploymentRate << "," << inflation << "\n";
    }
    auto stop = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = stop - start;

    cout << "Elapsed time: " << elapsed.count() << " seconds" << endl;
    cout << "... END program" << endl;

    summaryFile.close();
    return 0;
}
