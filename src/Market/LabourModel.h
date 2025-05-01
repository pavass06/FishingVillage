#ifndef LABOURMODEL_H
#define LABOURMODEL_H

#include <string>
#include "../Agent/FishingFirm.h"   // Chemin relatif vers votre FishingFirm.h

namespace LabourDemandModels {

    /// Règle simple : croissance de CA passée
    int computeJobsPastPerformance(
        const FishingFirm& firm,
        double growthThreshold = 0.10
    );

    /// Règle Eurace@Unibi : prévision → N* → ouvertures
    int computeJobsEUBI(
        const FishingFirm& firm,
        double alpha = 100.0
    );

    /// Wrapper modulaire : choisit la règle selon labourModelName
    int computeJobPostings(
        const FishingFirm& firm,
        const std::string& labourModelName,
        double growthThreshold = 0.10,
        double alpha = 100.0
    );

}  // namespace LabourDemandModels

#endif  // LABOURMODEL_H
