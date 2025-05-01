#include "LabourModel.h"
#include <cmath>       // pour std::ceil
#include <algorithm>   // pour std::max

namespace LabourDemandModels {

int computeJobsPastPerformance(
    const FirmState& firm,
    const WorldState& /*world*/,
    double growthThreshold
) {
    double prevRev = firm.previousRevenue;
    double currRev = firm.currentRevenue;
    if (prevRev <= 0) return 0;
    double growth = (currRev - prevRev) / prevRev;
    if (growth > growthThreshold * 2) return 2;
    if (growth > growthThreshold)     return 1;
    return 0;
}

int computeJobsEUBI(
    const FirmState& firm,
    const WorldState& /*world*/,
    double alpha
) {
    // 1) Prévision : moyenne des deux derniers revenus (exemple)
    double Ystar = 0.5 * (firm.currentRevenue + firm.previousRevenue);
    // 2) Emploi désiré
    int Nstar = static_cast<int>(std::ceil(Ystar / alpha));
    // 3) Poste à ouvrir = max(0, Nstar - effectif actuel)
    return std::max(0, Nstar - firm.currentEmployees);
}

int computeJobPostings(
    const FirmState& firm,
    const WorldState& world,
    const std::string& modelName,
    double growthThreshold,
    double alpha
) {
    if (modelName == "EUBI") {
        return computeJobsEUBI(firm, world, alpha);
    }
    // Par défaut ou "PastPerformance"
    return computeJobsPastPerformance(firm, world, growthThreshold);
}

}  // namespace LabourDemandModels
