#ifndef LABOURMODEL_H
#define LABOURMODEL_H

#include <string>
#include "World.h"        // ou le header où sont définis vos FirmState/WorldState

namespace LabourDemandModels {

    // Règle simple sur la performance passée
    int computeJobsPastPerformance(
        const FirmState& firm, 
        const WorldState& world, 
        double growthThreshold = 0.10
    );

    // Règle Eurace@Unibi plus réaliste
    int computeJobsEUBI(
        const FirmState& firm, 
        const WorldState& world, 
        double alpha = 100.0
    );

    // Wrapper qui choisit la règle selon un string
    int computeJobPostings(
        const FirmState& firm,
        const WorldState& world,
        const std::string& modelName,
        double growthThreshold = 0.10,
        double alpha = 100.0
    );

}  // namespace LabourDemandModels

#endif  // LABOURMODEL_H
