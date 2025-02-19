#ifndef FISHINGFIRM_H
#define FISHINGFIRM_H

#include "Firm.h"
#include <algorithm>
#include <iostream>
#include <cmath>             // For std::ceil
#include "FishingMarket.h"   // This file defines the struct FishOffering

class FishingFirm : public Firm {
public:
    // Constructor for FishingFirm.
    // PriceLevel is fixed to 6; jobPostMultiplier is set to 0.05.
    FishingFirm(int id, double initFunds, int lifetime, int numberOfEmployees,
                double stock,
                double salesEfficiency = 2.0, double jobPostMultiplier = 0.05)
        : Firm(id, initFunds, lifetime, numberOfEmployees, stock, 6.0, salesEfficiency, jobPostMultiplier)
    {}

    virtual ~FishingFirm() {}

    virtual double getGoodsSupply() const {
        return std::min(stock, salesEfficiency * static_cast<double>(numberOfEmployees));
    }

    // generateGoodsOffering(): Creates a FishOffering.
    virtual FishOffering generateGoodsOffering(double cost) const {
        FishOffering offer;
        offer.id = getID();
        offer.productSector = "fishing";
        offer.cost = cost;
        offer.offeredPrice = getPriceLevel();
        offer.quantity = getGoodsSupply();
        return offer;
    }

    // generateJobPosting(): Creates a JobPosting with fixed requirements (all set to 1).
    virtual JobPosting generateJobPosting(const std::string &sector, int eduReq, int expReq, int attract) const override {
    JobPosting posting;
    posting.firmID = getID();
    posting.jobSector = sector; // For our village, "fishing"
    posting.educationRequirement = eduReq;  // Fixed to 1.
    posting.experienceRequirement = expReq; // Fixed to 1.
    posting.attractiveness = attract;       // Fixed to 1.
    posting.vacancies = std::max(1, static_cast<int>(std::ceil(numberOfEmployees * jobPostMultiplier))); // Post at least 1 vacancy
    posting.recruiting = (posting.vacancies > 0);
    return posting;
}


    virtual void print() const override {
        Firm::print();
        std::cout << "Goods Supply (Fish Available): " << getGoodsSupply() << std::endl;
    }
};

#endif // FISHINGFIRM_H
