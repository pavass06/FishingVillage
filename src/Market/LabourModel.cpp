#include "LabourModel.h"
#include <cmath>      // std::ceil
#include <algorithm>  // std::max

namespace LabourDemandModels {

int computeJobsPastPerformance(
    const FishingFirm& firm,
    double growthThreshold
) {
    double prevRev = firm.getPreviousFirmRevenue();
    double currRev = firm.getCurrentFirmRevenue();
    if (prevRev <= 0) return 0;
    double growth = (currRev - prevRev) / prevRev;
    if (growth > 2 * growthThreshold) return 2;
    if (growth > growthThreshold)     return 1;
    return 0;
}

int computeJobsEUBI(
    const FishingFirm& firm,
    double alpha
) {
    // Exemple de prévision : moyenne des deux derniers revenus
    double Ystar = 0.5 * (firm.getCurrentFirmRevenue() + firm.getPreviousFirmRevenue());
    int Nstar = static_cast<int>(std::ceil(Ystar / alpha));
    int Ncurr = firm.getEmployeeCount();
    return std::max(0, Nstar - Ncurr);
}

int computeJobPostings(
    const FishingFirm& firm,
    const std::string& labourModelName,
    double growthThreshold,
    double alpha
) {
    if (labourModelName == "EUBI") {
        return computeJobsEUBI(firm, alpha);
    }
    // Par défaut ou "PastPerformance"
    return computeJobsPastPerformance(firm, growthThreshold);
}

}  // namespace LabourDemandModels
