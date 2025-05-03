#include <iostream>
#include <chrono>
#include <fstream>
#include <algorithm>
#include <vector>
#include <memory>
#include <random>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include "World.h"
#include "FishingFirm.h"
#include "FisherMan.h"
#include "JobMarket.h"
#include "FishingMarket.h"
#include "SimulationParameters.h"  // Contains SimulationParameters and parseParametersFromFile

using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <parameters_file>\n";
        return 1;
    }
    // Parse des paramètres
    //SimulationParameters params = parseParametersFromFile(argv[1]);
    SimulationParameters params = readParametersFromFile(argv[1]);

    // Création des marchés
    auto jobMarket = make_shared<JobMarket>(
        params,
        params.initialWage,
        params.perceivedPriceMean,
        /* mean fish order */ 1.0
    );
    auto fishingMarket = make_shared<FishingMarket>(
        params.perceivedPriceMean
    );

    //Création du monde avec la nouvelle signature
    World world(params,jobMarket,fishingMarket);

    // Build the vector of FishingFirms.
    vector<shared_ptr<FishingFirm>> firms;
    double initialStock = params.totalFisherMen / params.totalFirms; //names wrong??
    int totalEmployed = static_cast<int>(round(params.initialEmployed * params.totalFisherMen));

#if verbose
    printf(" Total Employed =%d Initial Stock=%f \n", totalEmployed,initialStock);
#endif    

    default_random_engine generator(static_cast<unsigned int>(time(nullptr)));
    normal_distribution<double> firmFundsDist(100.0, 20.0);
    // Deterministic pricing parameters
    double basePrice = 5.0;
    double stepPrice = 0.05;
    normal_distribution<double> fisherAgeDist(30, 20);
    normal_distribution<double> fisherLifetimeDist(60, 5);

    // Create each firm and set deterministic prices
    for (int id = 100, firmIdx = 0; id < 100 + params.totalFirms; id++, firmIdx++) {
        double funds = firmFundsDist(generator);
        int lifetime = 100000000;
        auto firm = make_shared<FishingFirm>(id, funds, lifetime,
                                             /*income=*/0, initialStock,
                                             params.employeeEfficiency);
        double price = basePrice + firmIdx * stepPrice;
        firm->setPriceLevel(price);
        cout << "[DEBUG] Initial price of firm " << firm->getID()
             << " set to " << price << endl;
        firms.push_back(firm);
    }
    world.setFirms(firms);

    // Provide the list of firms to the JobMarket.
    jobMarket->setFirmList(&firms);

    // Create fishermen.
    vector<shared_ptr<FisherMan>> employedFishers;
    for (int id = 0; id < params.totalFisherMen; id++) {
        double lifetimeYears = fisherLifetimeDist(generator);
        double lifetime = lifetimeYears * 365;
        double ageYears = fisherAgeDist(generator);
        double age = ageYears * 365;
        bool initiallyEmployed = (id < static_cast<int>(params.initialEmployed));
        auto fisher = make_shared<FisherMan>(
            id, /*initFunds=*/0.0, lifetime,
            /*income=*/age, /*savings=*/0.0,
            /*jobDemand=*/1.0, /*goodsDemand=*/1.0,
            /*firmID=*/0, /*wage=*/(initiallyEmployed ? params.initialWage : 0.0),
            /*unempBenefit=*/0.0,
            "fishing", /*edu*/1, /*exp*/1, /*pref*/1
        );
        world.addFisherMan(fisher);
        if (initiallyEmployed)
            employedFishers.push_back(fisher);
    }
    // Distribute employed fishermen across firms.
    for (size_t i = 0; i < employedFishers.size(); ++i) {
        firms[i % firms.size()]->addEmployee(employedFishers[i]);
    }

    cout << "BEGIN program ..." << endl;
    cout << "Days to simulate: " << params.totalCycles << endl;
    cout << "Initial number of fishers: " << params.totalFisherMen << endl;
    cout << "Calculated number of firms: " << params.totalFirms << endl;
    cout << "----------------------------------------------------------------------" << endl;

    ofstream summaryFile("economicdatas.csv");
    if (!summaryFile.is_open()) {
        cerr << "Error: Unable to open output file." << endl;
        return 1;
    }
    summaryFile << "Cycle,Year,DailyGDP,CyclyGDP,Population,GDPperCapita,Unemployment,Inflation\n";

    auto start = chrono::high_resolution_clock::now();
    // Dummy distribution to satisfy simulateCycle signature
    normal_distribution<double> unusedFirmPriceDist(0.0, 1.0);
    normal_distribution<double> localConsumerPriceDist(params.perceivedPriceMean, 0.8);
    uniform_int_distribution<int> goodsQuantityDist(1, 3);

    for (int day = 0; day < params.totalCycles; day++) {
        world.simulateCycle(generator, unusedFirmPriceDist, goodsQuantityDist, localConsumerPriceDist);

        int cycle = day + 1;
        double currentYear = cycle / params.cycleScale;
        double dailyGDP = world.getGDP();
        int totalFishers = world.getTotalFishers();
        double perCapita = (totalFishers > 0) ? (dailyGDP / totalFishers) : 0.0;
        double cyclyGDP = dailyGDP / params.cycleScale;
        double inflation = world.getInflation(day);
        double unemployment = world.getUnemployment(day);

        summaryFile << cycle << "," << currentYear << "," << dailyGDP << ","
                    << cyclyGDP << "," << totalFishers << ","
                    << perCapita << "," << unemployment << "," << inflation * 100 << "\n";
    }
    summaryFile.close();

    const vector<double>& unemploymentHistory = world.getUnemploymentHistory();
    ofstream unempFile("unemploymentHistory.csv");
    if (!unempFile.is_open()) {
        cerr << "Error: Unable to open unemployment output file." << endl;
        return 1;
    }
    unempFile << "Cycle,UnemploymentRate\n";
    for (size_t i = 0; i < unemploymentHistory.size(); i++) {
        unempFile << i+1 << "," << unemploymentHistory[i] * 100 << "\n";
    }
    unempFile.close();

    // Write firm revenue history to file. Is this neccesary? Too complicated
    int maxCycles = 0;
    for (const auto& firm : firms) {
        maxCycles = max(maxCycles, static_cast<int>(firm->getRevenueHistory().size()));
    }

    ofstream firmRevenueFile("firm_revenu.csv");
    if (!firmRevenueFile.is_open()) {
        cerr << "Error: Unable to open firm revenue output file." << endl;
        return 1;
    }

    // Header: IDs
    for (size_t i = 0; i < firms.size(); i++) {
        firmRevenueFile << firms[i]->getID();
        if (i < firms.size() - 1) firmRevenueFile << ",";
    }
    firmRevenueFile << "\n";

    // Revenues
    unsigned int maxCycles_sfe = static_cast<unsigned int>(maxCycles);
    for (unsigned int cycle = 0; cycle < maxCycles_sfe; cycle++) {
        for (size_t i = 0; i < firms.size(); i++) {
            const auto &hist = firms[i]->getRevenueHistory();
            double rev = (cycle < hist.size()) ? hist[cycle] : 0.0;
            firmRevenueFile << rev;
            if (i < firms.size() - 1) firmRevenueFile << ",";
        }
        firmRevenueFile << "\n";
    }
    firmRevenueFile.close();

    auto stop = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = stop - start;
    cout << "Elapsed time: " << elapsed.count() << " seconds" << endl;
    cout << "... END program" << endl;

    return 0;
}
