#ifndef SIMULATIONPARAMETERS_H
#define SIMULATIONPARAMETERS_H

#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <unordered_map>

using namespace std;

struct SimulationParameters {
    // 1. Test Configuration & Global Settings
    int totalCycles;                 // Total simulation cycles (days)
    double cycleScale;               // Number of days per year

    // 2. Population & Demographic Management
    int totalFisherMen;              // Total number of fishers
    double annualBirthRate;          // Annual birth rate (e.g., 0.02 for 2%)
    int maxStarvingDays;             // Consecutive days without fish before death
    double fisherAgeMean;            // Mean for initial age distribution (years)
    double fisherAgeVariance;        // Variance for age distribution
    double fisherLifetimeMean;       // Mean for lifetime distribution (years)
    double fisherLifetimeVariance;   // Variance for lifetime distribution

    // 3. Derived Population Fractions (fractions → counts after reading)
    double totalFirms;               // Fraction of population in firms
    double initialEmployed;          // Fraction of population initially employed
    double totalJobOffers;           // Fraction of population with job offers

    // 4. Economic Policy / Market Parameters
    double initialWage;              // Baseline wage / fish price reference
    double offeredPriceMean;         // Mean offered price by firms at start
    double offeredPriceVariance;     // Variance of offered‐price distribution
    double perceivedPriceMean;       // Mean perceived price by consumers at start
    double perceivedPriceVariance;   // Variance of perceived‐price distribution
    double employeeEfficiency;       // Fish caught per fisher per day

    // 4.1. Firm initial‐funds distribution
    double firmFundsDistMean;        // Mean of firm‐funds distribution
    double firmFundsVariance;        // Variance of firm‐funds distribution

    // 4.2. Firm lifetime
    int firmLifetime;                // Lifetime of firms (in days/cycles)

    // 5. Inflation Adjustment Parameters
    double meanAugmentationInflat;     // Mean factor when demand > supply
    double varianceAugmentationInflat; // Variance for augmentation factor
    double meanDiminutionInflat;       // Mean factor when supply > demand
    double varianceDiminutionInflat;    // Variance for diminution factor

    double postingRate;              // e.g., 0.1 (10% of current employees)
    double firingRate;               // e.g., 0.05 (5% of current employees)

    // 6. Labour model 
    string labourModel;
    double growthThreshold;   // Threshold for PastPerformance
    double alpha;             // Marginal productivity for EUBI

    // 7. Goods demand
    int goodsQuantityMin;      // Minimum goods quantity per consumer
    int goodsQuantityMax;      // Maximum goods quantity per consumer

     // 8. Fisher initial parameters
     double initialFisherFunds;   // Initial funds for each fisher
     double initialSavings;       // Initial savings for each fisher
     double initialJobDemand;     // Initial job‐demand factor
     double initialGoodsDemand;   // Initial goods‐demand factor
     double maxSalary;            // Salary cap per cycle 
};

/**
 * Reads a key-value file (each line “key value”, comments start with ‘#’)
 * and populates all fields above. Throws if any required key is missing.
 */
inline SimulationParameters readParametersFromFile(const std::string& filename) {
    SimulationParameters params;
    std::ifstream file(filename);
    if (!file.is_open())
        throw std::runtime_error("Could not open parameters file: " + filename);

    // Load all key→value pairs
    unordered_map<string,string> kv;
    string line;
    while (getline(file, line)) {
        if (auto pos = line.find('#'); pos != string::npos)
            line.resize(pos);
        istringstream iss(line);
        string key, value;
        if (iss >> key >> value)
            kv[key] = value;
    }

    try {
        // 1. Test Configuration
        params.totalCycles            = stoi(kv.at("TotalCycles"));
        params.cycleScale             = stod(kv.at("cycleScale"));

        // 2. Population & Demographics
        params.totalFisherMen         = stoi(kv.at("totalFisherMen"));
        params.annualBirthRate        = stod(kv.at("annualBirthRate"));
        params.maxStarvingDays        = stoi(kv.at("maxStarvingDays"));
        params.fisherAgeMean          = stod(kv.at("fisherAgeMean"));
        params.fisherAgeVariance      = stod(kv.at("fisherAgeVariance"));
        params.fisherLifetimeMean     = stod(kv.at("fisherLifetimeMean"));
        params.fisherLifetimeVariance = stod(kv.at("fisherLifetimeVariance"));

        // 3. Fractions → keep as fractions for now
        params.totalFirms             = stod(kv.at("totalFirms"));
        params.initialEmployed        = stod(kv.at("initialEmployed"));
        params.totalJobOffers         = stod(kv.at("totalJobOffers"));

        // 4. Economic Policy / Market
        params.initialWage            = stod(kv.at("initialWage"));
        params.offeredPriceMean       = stod(kv.at("offeredPriceMean"));
        params.offeredPriceVariance   = stod(kv.at("offeredPriceVariance"));
        params.perceivedPriceMean     = stod(kv.at("perceivedPriceMean"));
        params.perceivedPriceVariance = stod(kv.at("perceivedPriceVariance"));
        params.employeeEfficiency     = stod(kv.at("employeeEfficiency"));

        // 4.1. Firm funds & lifetime
        params.firmFundsDistMean      = stod(kv.at("firmFundsDistMean"));
        params.firmFundsVariance      = stod(kv.at("firmFundsVariance"));
        params.firmLifetime           = stoi(kv.at("firmLifetime"));

        // 5. Inflation
        params.meanAugmentationInflat   = stod(kv.at("meanAugmentationInflat"));
        params.varianceAugmentationInflat = stod(kv.at("varianceAugmentationInflat"));
        params.meanDiminutionInflat     = stod(kv.at("meanDiminutionInflat"));
        params.varianceDiminutionInflat = stod(kv.at("varianceDiminutionInflat"));

        params.postingRate            = stod(kv.at("postingRate"));
        params.firingRate             = stod(kv.at("firingRate"));

        // 6. Labour model
        params.labourModel            = kv.at("labourModel");
        params.growthThreshold        = stod(kv.at("growthThreshold"));
        params.alpha                  = stod(kv.at("alpha"));

        // 7. Goods demand
        params.goodsQuantityMin       = stoi(kv.at("goodsQuantityMin"));
        params.goodsQuantityMax       = stoi(kv.at("goodsQuantityMax"));

         // 8. Fisher initial parameters
        params.initialFisherFunds  = stod(kv.at("initialFisherFunds"));
        params.initialSavings      = stod(kv.at("initialSavings"));
        params.initialJobDemand    = stod(kv.at("initialJobDemand"));
        params.initialGoodsDemand  = stod(kv.at("initialGoodsDemand"));
        params.maxSalary           = stod(kv.at("maxSalary"));


    }
    catch (const out_of_range&) {
        throw runtime_error("Missing required parameter in input file.");
    }

    // Convert fractions → absolute counts
    params.totalFirms      = max(1, static_cast<int>(params.totalFirms * params.totalFisherMen));
    params.initialEmployed = static_cast<int>(params.initialEmployed * params.totalFisherMen);
    params.totalJobOffers  = static_cast<int>(params.totalJobOffers * params.totalFisherMen);

    return params;
}

#endif // SIMULATIONPARAMETERS_H
