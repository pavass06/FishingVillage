#ifndef FISHINGFIRM_H
#define FISHINGFIRM_H

#include "Firm.h"
#include <algorithm>
#include <iostream>
#include <cmath>  // For std::ceil
#include <memory>
#include <string>

// Si FishOffering n'est pas déjà défini, on le définit ici.
#ifndef FISH_OFFERING_DEFINED
#define FISH_OFFERING_DEFINED
struct FishOffering {
    int id;
    std::string productSector;
    double cost;
    double offeredPrice;
    double quantity;
    // Utilisation d'un shared_ptr pour pointer vers FishingFirm.
    std::shared_ptr<class FishingFirm> firm;
};
#endif

class FishingFirm : public Firm {
public:
    // Le priceLevel est fixé à 6; le jobPostMultiplier est fixé à 0.05.
    FishingFirm(int id, double initFunds, int lifetime, int numberOfEmployees,
                double stock,
                double salesEfficiency = 2.0, double jobPostMultiplier = 0.05)
        : Firm(id, initFunds, lifetime, numberOfEmployees, stock, 6.0, salesEfficiency, jobPostMultiplier)
    {}

    virtual ~FishingFirm() {}

    // Retourne la quantité de poisson disponible à la vente.
    virtual double getGoodsSupply() const {
        return std::min(stock, salesEfficiency * static_cast<double>(numberOfEmployees));
    }

    // Génère une offre de poisson.
    virtual FishOffering generateGoodsOffering(double cost) const {
        FishOffering offer;
        offer.id = getID();
        offer.productSector = "fishing";
        offer.cost = cost;
        offer.offeredPrice = getPriceLevel();
        offer.quantity = getGoodsSupply();
        // Le champ firm sera assigné par le code appelant (par exemple dans FishingMarket).
        return offer;
    }

    // Génère une offre d'emploi.
    virtual JobPosting generateJobPosting(const std::string &sector, int eduReq, int expReq, int attract) const override {
        JobPosting posting;
        posting.firmID = getID();
        posting.jobSector = sector; // Pour notre village, "fishing"
        posting.educationRequirement = eduReq;  
        posting.experienceRequirement = expReq; 
        posting.attractiveness = attract;       
        posting.vacancies = std::max(1, static_cast<int>(std::ceil(numberOfEmployees * jobPostMultiplier)));
        posting.recruiting = (posting.vacancies > 0);
        return posting;
    }

    virtual void print() const override {
        Firm::print();
        std::cout << "Goods Supply (Fish Available): " << getGoodsSupply() << std::endl;
    }
};

#endif // FISHINGFIRM_H
