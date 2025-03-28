#include <iostream>
#include <chrono>
#include <fstream>
#include <algorithm>
#include <random>
#include <cstdlib>
#include "World.h"
#include "FishingFirm.h"
#include "FisherMan.h"
#include "JobMarket.h"
#include "FishingMarket.h"
#include "SimulationParameters.h"  // Contient SimulationParameters et parseParametersFromFile

using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <parameters_file>" << endl;
        return 1;
    }
    // Lecture des paramètres.
    SimulationParameters params = parseParametersFromFile(argv[1]);

    // Création des marchés partagés.
    auto jobMarket = make_shared<JobMarket>(params.initialWage, params.perceivedPriceMean, 1);
    auto fishingMarket = make_shared<FishingMarket>(params.perceivedPriceMean);
    
    // Création de l'objet World.
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
                params.varianceDiminutionInflat,
                params.postingRate,
                params.firingRate);

    // Construction du vecteur de FishingFirm.
    vector<shared_ptr<FishingFirm>> firms;
    int initialStock = params.totalFisherMen / params.totalFirms;
    // Calcul d'un nombre initial d'employés par firme (au moins 1)
    int initialEmployeesPerFirm = max(1, static_cast<int>(params.initialEmployed / params.totalFisherMen * params.totalFisherMen / params.totalFirms));
    default_random_engine generator(static_cast<unsigned int>(time(0)));
    normal_distribution<double> firmFundsDist(100.0, 20.0);
    normal_distribution<double> firmPriceDist(params.offeredPriceMean, 0.5);
    normal_distribution<double> fisherAgeDist(30, 20);
    normal_distribution<double> fisherLifetimeDist(60, 5);
    for (int id = 100; id < 100 + params.totalFirms; id++) {
        double funds = firmFundsDist(generator);
        int lifetime = 100000000;
        auto firm = make_shared<FishingFirm>(id, funds, lifetime, 0, initialStock, params.employeeEfficiency);
        double price = firmPriceDist(generator);
        firm->setPriceLevel(price);
        firms.push_back(firm);
    }
    world.setFirms(firms);
    // Fournir la liste des firmes au JobMarket.
    jobMarket->setFirmList(&firms);

    // Création des pêcheurs.
    vector<shared_ptr<FisherMan>> employedFishers;
    vector<shared_ptr<FisherMan>> unemployedFishers;
    for (int id = 0; id < params.totalFisherMen; id++) {
        double lifetimeYears = fisherLifetimeDist(generator);
        double lifetime = lifetimeYears * 365;
        double ageYears = fisherAgeDist(generator);
        double age = ageYears * 365;
        // On crée tous les pêcheurs avec firmID = 0 par défaut.
        bool initiallyEmployed = (id < static_cast<int>(params.initialEmployed));
        auto fisher = make_shared<FisherMan>(
            id, 0.0, lifetime, age, 0.0, 1.0, 1.0,
            0, initiallyEmployed ? params.initialWage : 0.0,
            0.0, "fishing", 1, 1, 1
        );
        world.addFisherMan(fisher);
        if (initiallyEmployed)
            employedFishers.push_back(fisher);
        else
            unemployedFishers.push_back(fisher);
    }
    // Répartir les pêcheurs employés sur les firmes.
    int firmIndex = 0;
    int numFirms = firms.size();
    for (auto &fisher : employedFishers) {
        firms[firmIndex]->addEmployee(fisher);
        firmIndex = (firmIndex + 1) % numFirms;
    }

    cout << "BEGIN program ..." << endl;
    cout << "Days to simulate: " << params.totalCycles << endl;
    cout << "Initial number of fishers: " << params.totalFisherMen << endl;
    cout << "Calculated number of firms: " << params.totalFirms << endl;
    cout << "--------------------------" << endl;

    ofstream summaryFile("economicdatas.csv");
    if (!summaryFile.is_open()) {
        cerr << "Error: Unable to open output file." << endl;
        return 1;
    }
    summaryFile << "Cycle,Year,DailyGDP,CyclyGDP,Population,GDPperCapita,Unemployment,Inflation\n";

    auto start = chrono::high_resolution_clock::now();
    normal_distribution<double> localConsumerPriceDist(params.perceivedPriceMean, 0.8);
    uniform_int_distribution<int> goodsQuantityDist(1, 3);

    for (int day = 0; day < params.totalCycles; day++) {
        world.simulateCycle(generator, firmPriceDist, goodsQuantityDist, localConsumerPriceDist);

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

    auto stop = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = stop - start;
    cout << "Elapsed time: " << elapsed.count() << " seconds" << endl;
    cout << "... END program" << endl;

    summaryFile.close();
    return 0;
}
