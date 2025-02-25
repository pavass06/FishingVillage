#include <iostream>
#include <vector>
#include <memory>
#include <random>
#include <ctime>
#include <fstream>
#include <cstdlib>
#include "World.h"
#include "FishingFirm.h"
#include "FisherMan.h"
#include "JobMarket.h"
#include "FishingMarket.h"


double generateNormalRand(double mean, double variance) {
    static std::random_device rd;  // Seed for the random number engine
    static std::mt19937 gen(rd()); // Mersenne Twister PRNG
    double stddev = std::sqrt(variance); // Standard deviation is the square root of variance
    std::normal_distribution<double> distribution(mean, stddev);
    return distribution(gen); // Generate a random number from the normal distribution
}


using namespace std;

// Structure to hold simulation parameters.
struct SimulationParameters {
    int totalCycles = 100;          // 10 days
    int totalFisherMen = 10000;
    int totalFirms = 500;
    int initialEmployed = 90;       // 90 employed, 10 unemployed
    double initialWage = 5.0;       // Initial base wage
};

// The Simulation class encapsulates the entire simulation logic.
class Simulation {
private:
    SimulationParameters params;
    shared_ptr<JobMarket> jobMarket;
    shared_ptr<FishingMarket> fishingMarket;
    World world;
    vector<shared_ptr<FishingFirm>> firms;
    vector<shared_ptr<FisherMan>> fishers;
    
    // Random engine and distributions.
    default_random_engine generator;
    // For firms:
    normal_distribution<double> firmFundsDist;  // e.g., mean 100, std dev 20
    normal_distribution<double> firmStockDist;  // e.g., mean 50, std dev 10 ****
    normal_distribution<double> firmPriceDist;  // e.g., mean 6, std dev 0.5
    // For fishermen:
    normal_distribution<double> fisherLifetimeDist; // e.g., mean 60, std dev 5
    normal_distribution<double> fisherAgeDist; // e.g., mean 30, std dev 10
    // For fishing market consumer orders:
    normal_distribution<double> consumerPriceDist;
    uniform_int_distribution<int> goodsQuantityDist;
    
public:
    Simulation(const SimulationParameters &p)
    : params(p),
      jobMarket(make_shared<JobMarket>(p.initialWage)),
      fishingMarket(make_shared<FishingMarket>(6.0)),
      world(p.totalCycles, 0.1, jobMarket, fishingMarket),
      generator(static_cast<unsigned int>(time(0))),
      firmPriceDist(6.0, 0.5),
      consumerPriceDist(5.0, 0.8),
      goodsQuantityDist(1, 3)
    {
        // Initialize FishingFirms.
        for (int id = 100; id < 100 + params.totalFirms; id++) {
            int employees = 18;
           // double funds = firmFundsDist(generator);  // Firm funds drawn from N(100,20) 
            double funds = generateNormalRand(100,20);
            int lifetime = 365;  // For now, firm lifetime fixed (or you could use a distribution)
            //double stock = firmStockDist(generator);  // Stock drawn from N(50,10)

            double stock = generateNormalRand(50,10);
            //double stock = 50;
            double salesEff = 2.0;
            double jobMult = 0.05;  // Vacancies = ceil(employees*jobMult)
            double price = firmPriceDist(generator);
            auto firm = make_shared<FishingFirm>(id, funds, lifetime, employees, stock, salesEff, jobMult);
            firm->setPriceLevel(price);
            firms.push_back(firm);
            world.addFirm(firm);
        }    

        // Initialize FisherMen.
        // First 90 will be employed.
        for (int id = 0; id < params.initialEmployed; id++) {
            double funds = 0.0; // All fishermen start with 0 funds.
            double age = fisherAgeDist(generator); // Draw age from N(60,5)
            double lifetime = fisherLifetimeDist(generator); // Draw lifetime from N(60,5)
            auto fisher = make_shared<FisherMan>(id,             // id
        0.0,            // initFunds
        lifetime,       // lifetime
        0.0,            // income // Parameters useless for now
        0.0,            // savings
        1.0,            // jobDemand
        1.0,            // goodsDemand
        true,          // employed
        params.initialWage, // wage
        0.0,            // unemploymentBenefit
        std::string("fishing"),  // jobSector
        1,              // educationLevel
        1,              // experienceLevel
        1               // jobPreference
    );
            fishers.push_back(fisher);
            world.addFisherMan(fisher);
        }
        // Then, create the unemployed fishermen.
        for (int id = params.initialEmployed; id < params.totalFisherMen; id++) {
            double funds = 0.0;
            double age = fisherAgeDist(generator);
            double lifetime = fisherLifetimeDist(generator);
            auto fisher = make_shared<FisherMan>(id,             // id
        0.0,            // initFunds
        lifetime,       // lifetime
        0.0,            // income // Parameters useless for now
        0.0,            // savings
        1.0,            // jobDemand
        1.0,            // goodsDemand
        false,          // employed
        0.0,            // wage
        0.0,            // unemploymentBenefit
        std::string("fishing"),  // jobSector
        1,              // educationLevel
        1,              // experienceLevel
        1              // jobPreference);
    );

            fishers.push_back(fisher);
            world.addFisherMan(fisher);
        }
    }
    
    void run() {
        vector<double> GDPs;
        vector<double> unemploymentRates;
        vector<double> inflations;
        
        // Run the simulation loop.
        for (int day = 0; day < params.totalCycles; day++) {
            cout << "===== Day " << day + 1 << " =====" << endl;
            world.simulateCycle(generator, firmPriceDist, goodsQuantityDist, consumerPriceDist);
            
            // Compute GDP as the sum of revenues from all firms.
            // double dailyGDP = 0.0;
            // for (auto &firm : firms) {
            //     dailyGDP += firm->getRevenue();
            // }

            double dailyGDP = world.getGDP();

            // std::cout << " GDP = " << dailyGDP << std::endl;
            // exit(0); 

            GDPs.push_back(dailyGDP);
            
            // Compute unemployment rate from the world state:
            int totalFishers = world.getTotalFishers();
            int unemployedFishers = world.getUnemployedFishers();
            double dailyUnemploymentRate = (totalFishers > 0) ? (static_cast<double>(unemployedFishers) / totalFishers) * 100.0 : 0.0;
            unemploymentRates.push_back(dailyUnemploymentRate);
            
            static double prevFishPrice = fishingMarket->getClearingFishPrice();
            double currFishPrice = fishingMarket->getClearingFishPrice();
            double inflRate = (prevFishPrice > 0) ? (currFishPrice - prevFishPrice) / prevFishPrice : 0.0;
            inflations.push_back(inflRate);
            prevFishPrice = currFishPrice;
        }
        
        // Print the collected data.
        cout << "GDP Values = [";
        for (size_t i = 0; i < GDPs.size(); i++) {
            cout << GDPs[i];
            if (i != GDPs.size() - 1) cout << ", ";
        }
        cout << "]\n";
        
        cout << "Unemployment Values = [";
        for (size_t i = 0; i < unemploymentRates.size(); i++) {
            cout << unemploymentRates[i];
            if (i != unemploymentRates.size() - 1) cout << ", ";
        }
        cout << "]\n";
        
        cout << "Inflation Values = [";
        for (size_t i = 0; i < inflations.size(); i++) {
            cout << inflations[i];
            if (i != inflations.size() - 1) cout << ", ";
        }
        cout << "]\n";


        //// 

    }
};

int main() {
    SimulationParameters params;
    Simulation sim(params);
    sim.run();
    return 0;
}
