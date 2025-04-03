#ifndef SIMULATIONPARAMETERS_H
#define SIMULATIONPARAMETERS_H

#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>

#include "readkeyword.h"


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
   
    // read keywords
    parseKeyword(file, "totalCycles", params.totalCycles); 
    parseKeyword(file, "cycleScale", params.cycleScale); 
    // 2
    parseKeyword(file, "totalFisherMen", params.totalFisherMen);
    parseKeyword(file,"annualBirthRate", params.annualBirthRate);
    parseKeyword(file,"maxStarvingDays",params.maxStarvingDays);
    parseKeyword(file,"ageDistMean",params.ageDistMean);
    parseKeyword(file,"ageDistVariance",params.ageDistVariance);
    parseKeyword(file,"lifetimeDistMean",params.lifetimeDistMean);
    parseKeyword(file,"lifetimeDistVariance",params.lifetimeDistVariance);
    // 3
    parseKeyword(file,"totalFirms",params.totalFirms);
    parseKeyword(file,"initialEmployed",params.initialEmployed);
    parseKeyword(file,"totalJobOffers",params.totalJobOffers);
    // 4
    parseKeyword(file,"initialWage",params.initialWage);
    parseKeyword(file,"offeredPriceMean",params.offeredPriceMean);
    parseKeyword(file,"perceivedPriceMean",params.perceivedPriceMean);
    parseKeyword(file,"employeeEfficiency",params.employeeEfficiency);

#if debug    
   std::cout <<   " totalCycles " <<  params.totalCycles << std::endl;
   std::cout <<   " cycleScale  " <<  params.cycleScale << std::endl;
#endif
   

    // Conversion des paramètres fractionnaires en valeurs absolues.
    params.totalFirms = static_cast<int>(params.totalFirms * params.totalFisherMen);
    if (params.totalFirms < 1)
        params.totalFirms = 1;
    params.initialEmployed = static_cast<int>(params.initialEmployed * params.totalFisherMen);
    params.totalJobOffers = static_cast<int>(params.totalJobOffers * params.totalFisherMen);
    return params;
}





#endif // SIMULATIONPARAMETERS_H