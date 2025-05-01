#ifndef LABOURMODEL_H
#define LABOURMODEL_H

#include <string>

// forward‐declare FishingFirm so we can refer to it without including its header
class FishingFirm;

namespace LabourDemandModels {

    // simple past-performance rule
    int computeJobsPastPerformance(
        const FishingFirm& firm,
        double growthThreshold = 0.10
    );

    // Eurace@Unibi rule
    int computeJobsEUBI(
        const FishingFirm& firm,
        double alpha = 100.0
    );

    // wrapper that picks the rule by name
    int computeJobPostings(
        const FishingFirm& firm,
        const std::string& labourModelName,
        double growthThreshold = 0.10,
        double alpha = 100.0
    );

}  // namespace LabourDemandModels

#endif // LABOURMODEL_H
