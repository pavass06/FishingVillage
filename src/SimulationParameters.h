#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>

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
    double pQuit;                    // Daily probability that an employed fisher quits
    double employeeEfficiency;       // Fish caught per fisher per day

    // 5. Inflation Adjustment Parameters
    double meanAugmentationInflat;     // Mean factor when demand > supply (e.g., 1.025)
    double varianceAugmentationInflat;  // Variance for augmentation factor (e.g., 0.005)
    double meanDiminutionInflat;       // Mean factor when supply > demand (e.g., 0.975)
    double varianceDiminutionInflat;    // Variance for diminution factor (e.g., 0.005)
};

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
