// File: src/Agent/FishingFirm.h
#ifndef FISHINGFIRM_H
#define FISHINGFIRM_H

#include "Firm.h"
#include "FisherMan.h"
#include "../Market/JobTypes.h"         // JobPosting, JobApplication
#include "../Market/LabourModel.h"      // computeJobPostings

#include <vector>
#include <memory>
#include <algorithm>
#include <iostream>
#include <cmath>
#include <random>
#include <string>

#ifndef FISH_OFFERING_DEFINED
#define FISH_OFFERING_DEFINED

struct FishOffering {
    int id;
    std::string productSector;
    double cost;
    double offeredPrice;
    double quantity;
    std::shared_ptr<class FishingFirm> firm;
};
#endif

class FishingFirm : public Firm {
private:
    std::vector<std::shared_ptr<FisherMan>> employees;
    std::vector<double> revenueHistory;

public:
    // Constructor: priceLevel fixed to 6.0
    FishingFirm(int id,
                double initFunds,
                int lifetime,
                int initialEmployees,
                double stock,
                double salesEfficiency = 2.0)
        : Firm(id, initFunds, lifetime, initialEmployees, stock, 6.0, salesEfficiency, 0.0)
    {}

    virtual ~FishingFirm() = default;

    // Record revenue each cycle
    void recordRevenue() {
        revenueHistory.push_back(getRevenue());
    }
    const std::vector<double>& getRevenueHistory() const {
        return revenueHistory;
    }
    double getPreviousFirmRevenue() const {
        if (revenueHistory.size() >= 2)
            return revenueHistory[revenueHistory.size() - 2];
        return (!revenueHistory.empty() ? revenueHistory.back() : getRevenue());
    }
    double getCurrentFirmRevenue() const {
        return (!revenueHistory.empty() ? revenueHistory.back() : getRevenue());
    }

    int getEmployeeCount() const {
        return static_cast<int>(employees.size());
    }

    // Implements the base class pure virtual
    virtual JobPosting generateJobPosting(const std::string &sector,
                                          int eduReq,
                                          int expReq,
                                          int attract) const override {
        JobPosting posting;
        posting.firmID = getID();
        posting.jobSector = sector;
        posting.educationRequirement = eduReq;
        posting.experienceRequirement = expReq;
        posting.attractiveness = attract;
        posting.vacancies = 1;
        posting.recruiting = true;
        return posting;
    }

    // Modular job postings based on labourModelName
    std::vector<JobPosting> generateJobPostings(const std::string& labourModelName,
                                                double growthThreshold,
                                                double alpha) const {
        int nVacancies = LabourDemandModels::computeJobPostings(
            *this, labourModelName, growthThreshold, alpha
        );
        std::vector<JobPosting> vac;
        vac.reserve(nVacancies > 0 ? static_cast<std::vector<JobPosting>::size_type>(nVacancies) : 0);

        for (int i = 0; i < nVacancies; ++i) {
            // Default: call the single posting factory
            vac.push_back(generateJobPosting("fishing", 0, 0, 1));
        }
        return vac;
    }

    // Firing logic (unchanged)
    void generateFiring(double firstQuartile) {
        double firmRev = getCurrentFirmRevenue();
       // bool forceFire = (static_cast<double>(rand()) / RAND_MAX < 0.05);
        if (firmRev < firstQuartile) {
            int numToFire = static_cast<int>(std::ceil(std::log(firmRev + 1)));
            numToFire = std::min(numToFire, getEmployeeCount());
            std::shuffle(employees.begin(), employees.end(), std::default_random_engine(std::random_device{}()));
            for (int i = 0; i < numToFire && !employees.empty(); ++i) {
                employees.front()->setFirmID(0);
                employees.erase(employees.begin());
            }
        }
    }

    void addEmployee(std::shared_ptr<FisherMan> emp) {
        if (emp->getFirmID() == 0) {
            emp->setFirmID(getID());
            employees.push_back(emp);
            numberOfEmployees = getEmployeeCount();
        }
    }

    void removeEmployee(std::shared_ptr<FisherMan> emp) {
        auto it = std::find(employees.begin(), employees.end(), emp);
        if (it != employees.end()) {
            (*it)->setFirmID(0);
            employees.erase(it);
            numberOfEmployees = getEmployeeCount();
        }
    }

    virtual void print() const override {
        Firm::print();
        std::cout << "Employees: " << getEmployeeCount() << std::endl;
    }
};

#endif // FISHINGFIRM_H
