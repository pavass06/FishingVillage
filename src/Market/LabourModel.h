#ifndef LABOUR_DEMAND_MODELS_H
#define LABOUR_DEMAND_MODELS_H

#include <string>

struct WorldState {
    double economicGrowth; // Simplified example
};

struct FirmHistory {
    double lastRevenue;
    double prevRevenue;
    int employeeCount;
};

int computeJobsPastPerformance(const WorldState& world, const FirmHistory& firm);
int computeJobsEUBI(const WorldState& world, const FirmHistory& firm);
int computeJobPostings(const WorldState& world, const FirmHistory& firm, const std::string& modelType);

#endif
