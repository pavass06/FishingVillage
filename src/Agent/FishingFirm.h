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
#include <numeric> // For accumulate
#include <cstdlib> // For random_shuffle

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
    double revenue_; // Firm's revenue.
public:
    // Constructeur : priceLevel fixé à 6.0.
    FishingFirm(int id, double initFunds, int lifetime, int initialEmployees,
                double stock, double salesEfficiency = 2.0)
        : Firm(id, initFunds, lifetime, initialEmployees, stock, 6.0, salesEfficiency, 0.0),
          revenue_(0.0) // Initialize revenue_ to 0.
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
        // Optionnel : affecter l'entreprise (si shared_from_this() est applicable)
        // offer.firm = std::const_pointer_cast<FishingFirm>(shared_from_this());
        return offer;
    }

    // Génération d'une offre d'emploi simple.
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

    // Helper function: Compute the mean revenue from a vector of firm revenues.
    double computeMeanRevenue(const std::vector<double>& firmRevenues) const {
        double total = std::accumulate(firmRevenues.begin(), firmRevenues.end(), 0.0);
        return total / firmRevenues.size();
    }

    // Helper function: Compute the first quartile (25th percentile) of firm revenues.
    double computeFirstQuartile(const std::vector<double>& firmRevenues) const {
        std::vector<double> sortedRevenues = firmRevenues;
        std::sort(sortedRevenues.begin(), sortedRevenues.end());
        size_t n = sortedRevenues.size();
        size_t index = static_cast<size_t>(std::floor(0.25 * n));
        if (index >= n) {
            index = n - 1;
        }
        return sortedRevenues[index];
    }

    // Modified generateJobPostings function:
    // 1. Compute mean revenue.
    // 2. If this firm's revenue > mean, return ceil(log(revenue + 1)) postings.
    // 3. Otherwise, return 0 postings.
    std::vector<JobPosting> generateJobPostings(const std::vector<double>& allFirmRevenues,
                                                const std::string &sector,
                                                int eduReq,
                                                int expReq,
                                                int attract) const {
        std::vector<JobPosting> postings;
        double meanRevenue = computeMeanRevenue(allFirmRevenues);
        double firmRevenue = this->revenue_;
        int numPostings = 0;
        if (firmRevenue > meanRevenue) {
            numPostings = static_cast<int>(std::ceil(std::log(firmRevenue + 1)));
        }
        for (int i = 0; i < numPostings; i++) {
            postings.push_back(generateJobPosting(sector, eduReq, expReq, attract));
        }
        return postings;
    }

    // New function: generateFiring.
    // It follows these rules:
    // 1. Compute mean revenue and first quartile revenue.
    // 2. If this firm's revenue < first quartile, fire randomly ceil(log(revenue + 1)) employees.
    // 3. Otherwise, do nothing.
    void generateFiring(const std::vector<double>& allFirmRevenues) {
        double meanRevenue = computeMeanRevenue(allFirmRevenues);
        double firstQuartileRevenue = computeFirstQuartile(allFirmRevenues);
        double firmRevenue = this->revenue_;
        int numToFire = 0;
        if (firmRevenue < firstQuartileRevenue) {
            numToFire = static_cast<int>(std::ceil(std::log(firmRevenue + 1)));
            if (numToFire > static_cast<int>(employees.size())) {
                numToFire = employees.size();
            }
            // Randomize the order of employees.
            std::shuffle(employees.begin(), employees.end());
            // Fire the first 'numToFire' employees from the shuffled vector.
            for (int i = 0; i < numToFire; i++) {
                if (!employees.empty())
                    removeEmployee(employees.front());
            }
        }
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

    // Licencie un nombre donné d'employés (en retirant de la fin).
    void fireEmployees(int count) {
        for (int i = 0; i < count && !employees.empty(); i++) {
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
