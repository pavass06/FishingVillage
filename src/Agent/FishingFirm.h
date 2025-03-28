#ifndef FISHINGFIRM_H
#define FISHINGFIRM_H

#include "Firm.h"
#include <algorithm>
#include <iostream>
#include <cmath>      // For std::floor
#include <memory>
#include <string>

// If FishOffering is not already defined, define it here.
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
public:
    // Constructor: priceLevel is fixed at 6.0.
    // We no longer use jobPostMultiplier.
    FishingFirm(int id, double initFunds, int lifetime, int numberOfEmployees,
                double stock,
                double salesEfficiency = 2.0)
        : Firm(id, initFunds, lifetime, numberOfEmployees, stock, 6.0, salesEfficiency, 0.0)
    {}

    virtual ~FishingFirm() {}

    // Return the quantity of fish available for sale.
    virtual double getGoodsSupply() const {
        // Ensure only whole fish are considered.
        return std::min(stock, salesEfficiency * static_cast<double>(numberOfEmployees));
    }

    // Generate a fish offering.
    virtual FishOffering generateGoodsOffering(double cost) const {
        FishOffering offer;
        offer.id = getID();
        offer.productSector = "fishing";
        offer.cost = cost;
        offer.offeredPrice = getPriceLevel();
        offer.quantity = getGoodsSupply();
        // The 'firm' field will be set externally.
        return offer;
    }

    // Generate a job posting.
    virtual JobPosting generateJobPosting(const std::string &sector, int eduReq, int expReq, int attract) const override {
        JobPosting posting;
        posting.firmID = getID();
        posting.jobSector = sector; // For our village, this is "fishing"
        posting.educationRequirement = eduReq;
        posting.experienceRequirement = expReq;
        posting.attractiveness = attract;
        // Return a default vacancy count of 1.
        posting.vacancies = 1;
        posting.recruiting = (posting.vacancies > 0);
        return posting;
    }


    // New method: generate multiple job postings based on posting_rate.
    std::vector<JobPosting> generateJobPostings(double posting_rate, const std::string &sector, int eduReq, int expReq, int attract) const {
        std::vector<JobPosting> postings;
        int currentEmployees = numberOfEmployees; // Using the inherited variable.
        int numPostings = static_cast<int>(std::ceil(posting_rate * currentEmployees));
        for (int i = 0; i < numPostings; i++) {
            postings.push_back(generateJobPosting(sector, eduReq, expReq, attract));
        }
        return postings;
    }

    // New method: fire a number of employees.
    void fireEmployees(int count) {
        numberOfEmployees = std::max(0, numberOfEmployees - count);
    }
    
    // (Assuming you have or can add a getter for the employee count.)
    int getEmployeeCount() const { return numberOfEmployees; }

    virtual void print() const override {
        Firm::print();
        std::cout << "Goods Supply (Fish Available): " << getGoodsSupply() << std::endl;
    }
};

#endif // FISHINGFIRM_H
