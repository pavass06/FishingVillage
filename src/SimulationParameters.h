#ifndef SIMULATIONPARAMETERS_H
#define SIMULATIONPARAMETERS_H

#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>


using namespace std;

struct SimulationParameters {
    // 1. Test Configuration & Global Settings
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
    double employeeEfficiency;       // Fish caught per fisher per day

    // 5. Inflation Adjustment Parameters
    double meanAugmentationInflat;     // Mean factor when demand > supply (e.g., 1.025)
    double varianceAugmentationInflat;  // Variance for augmentation factor (e.g., 0.005)
    double meanDiminutionInflat;       // Mean factor when supply > demand (e.g., 0.975)
    double varianceDiminutionInflat;    // Variance for diminution factor (e.g., 0.005)

    double postingRate;              // e.g., 0.1 (10% of current employees)
    double firingRate;               // e.g., 0.05 (5% of current employees)
};

SimulationParameters parseParametersFromFile(const std::string &filename) {
    SimulationParameters params;
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open parameters file.");
    }
    std::string line;
    int lineNumber = 0;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::istringstream iss(line);
        lineNumber++;
        switch(lineNumber) {
            case 1:  iss >> params.totalCycles; break;
            case 2:  iss >> params.cycleScale; break;
            case 3:  iss >> params.totalFisherMen; break;
            case 4:  iss >> params.annualBirthRate; break;
            case 5:  iss >> params.maxStarvingDays; break;
            case 6:  iss >> params.ageDistMean; break;
            case 7:  iss >> params.ageDistVariance; break;
            case 8:  iss >> params.lifetimeDistMean; break;
            case 9:  iss >> params.lifetimeDistVariance; break;
            case 10: iss >> params.totalFirms; break;
            case 11: iss >> params.initialEmployed; break;
            case 12: iss >> params.totalJobOffers; break;
            case 13: iss >> params.initialWage; break;
            case 14: iss >> params.offeredPriceMean; break;
            case 15: iss >> params.perceivedPriceMean; break;
            case 16: iss >> params.employeeEfficiency; break;
            case 17: iss >> params.meanAugmentationInflat; break;
            case 18: iss >> params.varianceAugmentationInflat; break;
            case 19: iss >> params.meanDiminutionInflat; break;
            case 20: iss >> params.varianceDiminutionInflat; break;
            case 21: iss >> params.postingRate; break;
            case 22: iss >> params.firingRate; break;
            default: break;
        }
    }
    // Conversion des paramètres fractionnaires en valeurs absolues.
    params.totalFirms = static_cast<int>(params.totalFirms * params.totalFisherMen);
    if (params.totalFirms < 1)
        params.totalFirms = 1;
    params.initialEmployed = static_cast<int>(params.initialEmployed * params.totalFisherMen);
    params.totalJobOffers = static_cast<int>(params.totalJobOffers * params.totalFisherMen);
    return params;
}

#endif // SIMULATIONPARAMETERS_H