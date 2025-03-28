#ifndef FISHINGFIRM_H
#define FISHINGFIRM_H

#include "Firm.h"
#include "FisherMan.h"
#include "JobMarket.h"  // For JobPosting struct

#include <algorithm>
#include <iostream>
#include <cmath>
#include <memory>
#include <string>
#include <vector>

#ifndef FISH_OFFERING_DEFINED
#define FISH_OFFERING_DEFINED

class FisherMan;

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
    // Liste des employés (pointeurs partagés vers FisherMan)
    std::vector<std::shared_ptr<FisherMan>> employees;
public:
    // Constructeur : priceLevel fixé à 6.0.
    FishingFirm(int id, double initFunds, int lifetime, int initialEmployees,
                double stock, double salesEfficiency = 2.0)
        : Firm(id, initFunds, lifetime, initialEmployees, stock, 6.0, salesEfficiency, 0.0)
    {
        // La liste employees sera remplie par la suite lors de l'initialisation.
    }

    virtual ~FishingFirm() {}

    // Renvoie la quantité de poissons disponibles à la vente.
    virtual double getGoodsSupply() const {
        return std::min(stock, salesEfficiency * static_cast<double>(employees.size()));
    }

    virtual FishOffering generateGoodsOffering(double cost) const {
        FishOffering offer;
        offer.id = getID();
        offer.productSector = "fishing";
        offer.cost = cost;
        offer.offeredPrice = getPriceLevel();
        offer.quantity = getGoodsSupply();
        return offer;
    }

    // Génération d'une offre d'emploi.
    virtual JobPosting generateJobPosting(const std::string &sector, int eduReq, int expReq, int attract) const override {
        JobPosting posting;
        posting.firmID = getID();
        posting.jobSector = sector;
        posting.educationRequirement = eduReq;
        posting.experienceRequirement = expReq;
        posting.attractiveness = attract;
        posting.vacancies = 1;
        posting.recruiting = (posting.vacancies > 0);
        return posting;
    }

    // Génère plusieurs offres d'emploi en fonction du posting_rate.
    std::vector<JobPosting> generateJobPostings(double posting_rate, const std::string &sector, int eduReq, int expReq, int attract) const {
        std::vector<JobPosting> postings;
        int currentEmployees = employees.size();
        int numPostings = static_cast<int>(std::ceil(posting_rate * currentEmployees));
        for (int i = 0; i < numPostings; i++) {
            postings.push_back(generateJobPosting(sector, eduReq, expReq, attract));
        }
        return postings;
    }

    // Ajoute un pêcheur aux employés et met à jour son firmID.
    void addEmployee(std::shared_ptr<FisherMan> emp) {
        employees.push_back(emp);
        emp->setFirmID(this->getID());
        numberOfEmployees = employees.size();
    }

    // Retire un pêcheur des employés et réinitialise son firmID à 0.
    void removeEmployee(std::shared_ptr<FisherMan> emp) {
        auto it = std::find(employees.begin(), employees.end(), emp);
        if (it != employees.end()) {
            emp->setFirmID(0);
            employees.erase(it);
            numberOfEmployees = employees.size();
        }
    }

    // Licencie un nombre donné d'employés.
    void fireEmployees(int count) {
        for (int i = 0; i < count && !employees.empty(); i++) {
            // Retire le dernier employé (par exemple).
            std::shared_ptr<FisherMan> emp = employees.back();
            emp->setFirmID(0);
            employees.pop_back();
        }
        numberOfEmployees = employees.size();
    }
    
    // Renvoie le nombre d'employés.
    int getEmployeeCount() const { 
        return employees.size();
    }

    virtual void print() const override {
        Firm::print();
        std::cout << "Goods Supply (Fish Available): " << getGoodsSupply() << std::endl;
        std::cout << "Nombre d'employés: " << getEmployeeCount() << std::endl;
    }
};

#endif // FISHINGFIRM_H
