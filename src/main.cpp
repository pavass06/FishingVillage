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
#include "SimulationParameters.h"  // Contient la définition de SimulationParameters et parseParametersFromFile

using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <parameters_file>" << endl;
        return 1;
    }
    // Lecture des paramètres de simulation.
    SimulationParameters params = parseParametersFromFile(argv[1]);

    // Création des marchés partagés.
    auto jobMarket = make_shared<JobMarket>(params.initialWage, params.perceivedPriceMean, 1);
    auto fishingMarket = make_shared<FishingMarket>(params.perceivedPriceMean);

    // Création de l'objet World avec posting et firing.
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
                params.postingRate,   // Posting rate (ex: 0.10)
                params.firingRate);   // Firing rate (ex: 0.05)

    // Construction du vecteur de FishingFirm.
    vector<shared_ptr<FishingFirm>> firms;
    int initialStock = params.totalFisherMen / params.totalFirms;
    int initialEmployeesPerFirm = max(1, static_cast<int>(params.initialEmployed / params.totalFirms));
    default_random_engine generator(static_cast<unsigned int>(time(0)));
    normal_distribution<double> firmFundsDist(100.0, 20.0);
    normal_distribution<double> firmPriceDist(params.offeredPriceMean, 0.5);
    normal_distribution<double> fisherAgeDist(30, 20);      // Âge moyen 30 ans, écart type 20 ans.
    normal_distribution<double> fisherLifetimeDist(60, 5);    // Durée de vie moyenne 60 ans, écart type 5 ans.
    for (int id = 100; id < 100 + params.totalFirms; id++) {
        double funds = firmFundsDist(generator);
        int lifetime = 100000000; // Durée de vie (jours)
        auto firm = make_shared<FishingFirm>(id, funds, lifetime, initialEmployeesPerFirm, initialStock, params.employeeEfficiency);
        double price = firmPriceDist(generator);
        firm->setPriceLevel(price);
        firms.push_back(firm);
    }
    world.setFirms(firms);

    // Création et ajout des FisherMan.
    // Pêcheurs employés.
    for (int id = 0; id < params.initialEmployed; id++) {
        double lifetimeYears = fisherLifetimeDist(generator);
        double lifetime = lifetimeYears * 365; // Conversion en jours.
        double ageYears = fisherAgeDist(generator);
        double age = ageYears * 365; // Conversion en jours.
        auto fisher = make_shared<FisherMan>(
            id, 0.0, lifetime, age, 0.0, 1.0, 1.0, true,
            params.initialWage, 0.0, "fishing", 1, 1, 1
        );
        world.addFisherMan(fisher);
    }
    // Pêcheurs au chômage.
    for (int id = params.initialEmployed; id < params.totalFisherMen; id++) {
        double lifetimeYears = fisherLifetimeDist(generator);
        double lifetime = lifetimeYears * 365;
        double ageYears = fisherAgeDist(generator);
        double age = ageYears * 365;
        auto fisher = make_shared<FisherMan>(
            id, 0.0, lifetime, age, 0.0, 1.0, 1.0, false,
            0.0, 0.0, "fishing", 1, 1, 1
        );
        world.addFisherMan(fisher);
    }

    cout << "BEGIN program ..." << endl;
    cout << "Days to simulate: " << params.totalCycles << endl;
    cout << "Initial number of fishers: " << params.totalFisherMen << endl;
    cout << "Calculated number of firms: " << params.totalFirms << endl;
    cout << "--------------------------" << endl;

    // Ouverture du fichier de sortie pour les données économiques.
    ofstream summaryFile("economicdatas.csv");
    if (!summaryFile.is_open()) {
        cerr << "Error: Unable to open output file." << endl;
        return 1;
    }
    // En-tête du fichier CSV.
    summaryFile << "Cycle,Year,DailyGDP,CyclyGDP,Population,GDPperCapita,Unemployment,Inflation\n";

    auto start = chrono::high_resolution_clock::now();
    normal_distribution<double> localConsumerPriceDist(params.perceivedPriceMean, 0.8);
    uniform_int_distribution<int> goodsQuantityDist(1, 3);

    // Boucle de simulation pour chaque cycle.
    for (int day = 0; day < params.totalCycles; day++) {
        world.simulateCycle(generator, firmPriceDist, goodsQuantityDist, localConsumerPriceDist);

        int cycle = day + 1;
        double currentYear = cycle / params.cycleScale;
        double dailyGDP = world.getGDP();
        int totalFishers = world.getTotalFishers();
        double perCapita = (totalFishers > 0) ? (dailyGDP / totalFishers) : 0.0;
        double cyclyGDP = dailyGDP / params.cycleScale;
        double inflation = world.getInflation(day);
        double unemployment = world.getUnemployment(day); // Récupération du taux de chômage pour ce cycle.

        summaryFile << cycle << "," << currentYear << "," << dailyGDP << ","
                    << cyclyGDP << "," << totalFishers << ","
                    << perCapita << "," << unemployment << "," << inflation * 100 << "\n";
    }

    // Vous pouvez également enregistrer l'historique complet dans un fichier séparé.
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
