#include "LabourModel.h"
#include <cmath>
#include <algorithm>

int computeJobsPastPerformance(const WorldState& world, const FirmHistory& firm) {
    double growth = firm.lastRevenue - firm.prevRevenue;
    if (growth > 0.1 * firm.prevRevenue) {
        return 2;
    } else if (growth > 0.0) {
        return 1;
    }
    return 0;
}

int computeJobsEUBI(const WorldState& world, const FirmHistory& firm) {
    double forecast = 0.5 * firm.lastRevenue + 0.5 * firm.prevRevenue;
    double Y_star = forecast;
    double alpha = 100.0;
    int N_star = static_cast<int>(std::ceil(Y_star / alpha));
    int N_current = firm.employeeCount;
    return std::max(N_star - N_current, 0);
}

int computeJobPostings(const WorldState& world, const FirmHistory& firm, const std::string& modelType) {
    if (modelType == "PastPerformance") {
        return computeJobsPastPerformance(world, firm);
    } else if (modelType == "EUBI") {
        return computeJobsEUBI(world, firm);
    }
    return 0;
}
