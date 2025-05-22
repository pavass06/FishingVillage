#include "LabourModel.h"
#include "../Agent/FishingFirm.h" 
#include <cmath>      // std::ceil
#include <algorithm>  // std::max


using namespace std;

namespace LabourDemandModels {

    int computeJobsPastPerformance(
        const FishingFirm& firm,
        double growthThreshold
    ) {
        double prevRev = firm.getPreviousFirmRevenue();
        double currRev = firm.getCurrentFirmRevenue();
        printf(" firm=%d \n",firm.getID());
        printf(" getCurrentFirmRevenue= %f \n", firm.getCurrentFirmRevenue());
        printf(" getPreviousFirmRevenue= %f \n", firm.getPreviousFirmRevenue());
        printf(" ------ \n");
        prevRev=std::max(prevRev,1e-6);
        double growth = (currRev - prevRev) / prevRev;
        if (growth > 2*growthThreshold) return 2; //hire
        if (growth > growthThreshold)   return 1;
        if (growth < -2*growthThreshold) return -2; //fire
        if (growth < -growthThreshold)   return -1;
        return 0;
    }
    
    int computeJobsEUBI(
        const FishingFirm& firm,
        double alpha
    ) {
        double Ystar = 0.5 * (firm.getCurrentFirmRevenue()
                              + firm.getPreviousFirmRevenue());
        std::cout << " prev revenue = " << firm.getPreviousFirmRevenue() << std::endl;
        std::cout << " current revenue = " << firm.getCurrentFirmRevenue() << std::endl;
        int Nstar = static_cast<int>(std::ceil(Ystar / alpha));
        int Ncurr = firm.getEmployeeCount();
        return Nstar - Ncurr;
    }
    
    int computeJobPostings(
        const FishingFirm& firm,
        const std::string& labourModelName,
        double growthThreshold,
        double alpha
    ) {
        if (labourModelName == "EUBI")
            return computeJobsEUBI(firm, alpha);
        return computeJobsPastPerformance(firm, growthThreshold);
    }
    
    } // namespace LabourDemandModels